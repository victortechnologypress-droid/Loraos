/* Adauga la sfarsitul window.c */
/* window_create - wrapper simplu, folosit din desktop */
void window_create(const char* title, int x, int y, int w, int h)
{
    /* Creat extern din desktop.c via window_create_desktop */
    (void)title; (void)x; (void)y; (void)w; (void)h;
}
