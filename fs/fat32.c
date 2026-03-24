/* fs/fat32.c
 * LoraOS - Driver FAT32 pentru persistenta pe USB
 * Permite citirea/scrierea fisierelor pe stick-ul de boot
 *
 * Arhitectura:
 * USB stick (FAT32) --> ATA/ATAPI read --> Parse FAT32 --> File ops
 */

#include "../kernel/include/kernel.h"
#include "fat32.h"

/* ---- Porturi ATA (Primary channel) ---- */
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECCOUNT    0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_STATUS      0x1F7
#define ATA_CMD         0x1F7

/* ---- Structuri FAT32 ---- */

/* Boot sector FAT32 (primii 512 bytes ai partitiei) */
typedef struct __attribute__((packed)) {
    uint8_t  jump[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;      /* De obicei 512 */
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;      /* Sectoare rezervate (contine boot sector) */
    uint8_t  num_fats;              /* De obicei 2 */
    uint16_t root_entry_count;      /* 0 pentru FAT32 */
    uint16_t total_sectors_16;      /* 0 pentru FAT32 */
    uint8_t  media_type;
    uint16_t fat_size_16;           /* 0 pentru FAT32 */
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    /* FAT32 extended */
    uint32_t fat_size_32;           /* Sectoare per FAT */
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;          /* De obicei 2 */
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;        /* 0x29 */
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];            /* "FAT32   " */
} fat32_boot_t;

/* Entry de director FAT32 (32 bytes) */
typedef struct __attribute__((packed)) {
    uint8_t  name[8];
    uint8_t  ext[3];
    uint8_t  attributes;
    uint8_t  reserved;
    uint8_t  create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;          /* Partea superioara a cluster-ului */
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t cluster_low;           /* Partea inferioara */
    uint32_t file_size;
} fat32_entry_t;

/* ---- Stare globala FAT32 ---- */
static fat32_boot_t boot_sector;
static uint8_t      fat32_ready = FALSE;
static uint32_t     fat_start_lba = 0;
static uint32_t     data_start_lba = 0;
static uint32_t     root_cluster = 2;

/* ============================================================
 *  ATA: Citeste un sector (512 bytes) de la adresa LBA
 * ============================================================ */
static int ata_read_sector(uint32_t lba, uint8_t* buffer)
{
    /* Asteapta ca drive-ul sa fie gata */
    int timeout = 100000;
    while ((inb(ATA_STATUS) & 0x80) && --timeout); /* BSY flag */
    if (!timeout) return -1;

    /* Selecteaza drive-ul si LBA mode */
    outb(ATA_DRIVE,    0xE0 | ((lba >> 24) & 0x0F)); /* Drive 0, LBA mode */
    outb(ATA_SECCOUNT, 1);                            /* Citeste 1 sector */
    outb(ATA_LBA_LOW,  (uint8_t)(lba));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_CMD,      0x20); /* READ SECTORS command */

    /* Asteapta DRQ (Data Request) */
    timeout = 100000;
    while (!(inb(ATA_STATUS) & 0x08) && --timeout);
    if (!timeout) return -1;

    /* Citeste 256 words (512 bytes) */
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(ATA_DATA);
    }

    return 0;
}

/* ============================================================
 *  ATA: Scrie un sector
 * ============================================================ */
static int ata_write_sector(uint32_t lba, const uint8_t* buffer)
{
    int timeout = 100000;
    while ((inb(ATA_STATUS) & 0x80) && --timeout);
    if (!timeout) return -1;

    outb(ATA_DRIVE,    0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)(lba));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_CMD,      0x30); /* WRITE SECTORS command */

    timeout = 100000;
    while (!(inb(ATA_STATUS) & 0x08) && --timeout);
    if (!timeout) return -1;

    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA, buf16[i]);
    }

    /* Flush cache */
    outb(ATA_CMD, 0xE7);
    timeout = 100000;
    while ((inb(ATA_STATUS) & 0x80) && --timeout);

    return 0;
}

/* ============================================================
 *  fat32_init - Parseaza boot sector-ul si initializeaza FAT32
 * ============================================================ */
void fat32_init(void)
{
    uint8_t sector_buf[512];

    /* Citeste boot sector-ul (LBA 0 = primul sector al partitiei)
     * Nota: Pe un USB cu o singura partitie, MBR e la LBA 0,
     * partitia incepe la LBA specificat in partition table */

    /* Simplu: citim LBA 0 (MBR) pentru a gasi partitia FAT32 */
    if (ata_read_sector(0, sector_buf) != 0) {
        fat32_ready = FALSE;
        return;
    }

    /* Verifica semnatura MBR (0x55AA la offset 510) */
    if (sector_buf[510] != 0x55 || sector_buf[511] != 0xAA) {
        fat32_ready = FALSE;
        return;
    }

    /* Partition table entry 1 (la offset 0x1BE) */
    uint32_t partition_lba = *((uint32_t*)(sector_buf + 0x1C6));

    /* Citeste boot sector-ul partitiei FAT32 */
    if (ata_read_sector(partition_lba, sector_buf) != 0) {
        fat32_ready = FALSE;
        return;
    }

    /* Copiaza in structura noastra */
    kmemcpy(&boot_sector, sector_buf, sizeof(fat32_boot_t));

    /* Calculeaza adresele de start */
    fat_start_lba  = partition_lba + boot_sector.reserved_sectors;
    data_start_lba = fat_start_lba + boot_sector.num_fats * boot_sector.fat_size_32;
    root_cluster   = boot_sector.root_cluster;

    fat32_ready = TRUE;
}

/* ============================================================
 *  fat32_is_ready - Verifica daca FAT32 e initializat
 * ============================================================ */
int fat32_is_ready(void)
{
    return fat32_ready;
}

/* ============================================================
 *  fat32_cluster_to_lba - Converteste cluster -> adresa LBA
 * ============================================================ */
static uint32_t cluster_to_lba(uint32_t cluster)
{
    return data_start_lba + (cluster - 2) * boot_sector.sectors_per_cluster;
}

/* ============================================================
 *  fat32_read_file - Citeste un fisier dupa nume (8.3 format)
 *  Returneaza numarul de bytes cititi, sau -1 la eroare
 * ============================================================ */
int fat32_read_file(const char* filename, uint8_t* buffer, uint32_t max_size)
{
    if (!fat32_ready) return -1;

    uint8_t sector_buf[512];
    uint32_t lba = cluster_to_lba(root_cluster);

    /* Citeste primul sector al directorului root */
    if (ata_read_sector(lba, sector_buf) != 0) return -1;

    /* Cauta fisierul in director */
    fat32_entry_t* entries = (fat32_entry_t*)sector_buf;
    for (int i = 0; i < 512 / 32; i++) {
        if (entries[i].name[0] == 0x00) break;  /* Sfarsit director */
        if (entries[i].name[0] == 0xE5) continue; /* Deleted */
        if (entries[i].attributes & 0x08) continue; /* Volume label */

        /* Compara numele (format 8.3: "FILE    EXT") */
        /* TODO: Implementare completa de matching */
        (void)filename;

        /* Calculeaza cluster-ul de start al fisierului */
        uint32_t file_cluster = ((uint32_t)entries[i].cluster_high << 16)
                              | entries[i].cluster_low;
        uint32_t file_size    = entries[i].file_size;

        if (file_size > max_size) file_size = max_size;

        /* Citeste datele fisierului cluster cu cluster */
        uint32_t bytes_read = 0;
        uint32_t current_cluster = file_cluster;

        while (current_cluster < 0x0FFFFFF8 && bytes_read < file_size) {
            uint32_t file_lba = cluster_to_lba(current_cluster);

            for (uint8_t s = 0; s < boot_sector.sectors_per_cluster && bytes_read < file_size; s++) {
                if (ata_read_sector(file_lba + s, sector_buf) != 0) return bytes_read;

                uint32_t to_copy = 512;
                if (bytes_read + to_copy > file_size) to_copy = file_size - bytes_read;

                kmemcpy(buffer + bytes_read, sector_buf, to_copy);
                bytes_read += to_copy;
            }

            /* Urmatorul cluster din FAT */
            uint32_t fat_offset = current_cluster * 4;
            uint32_t fat_sector = fat_start_lba + fat_offset / 512;
            uint32_t fat_entry_off = fat_offset % 512;

            if (ata_read_sector(fat_sector, sector_buf) != 0) break;
            current_cluster = *((uint32_t*)(sector_buf + fat_entry_off)) & 0x0FFFFFFF;
        }

        return (int)bytes_read;
    }

    return -1; /* Fisier negasit */
}
