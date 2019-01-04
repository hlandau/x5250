/* Temporary function to fill the buffers with test data.  Fills both
 * buf5250 and fields5250.  This should be replaced later with a
 * function that reads the 5250 data stream.
 */
int fillbuf (int instance, int subwindowcount);
void dump_buffer (int bufferid);
void dump_resource_database (XrmDatabase resourceDB, char *appname);
int is_font_scalable (char *name);
int font_point_size (char *name);
