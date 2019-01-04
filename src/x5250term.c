/*
 * Copyright 2004 James Rich
 *
 * This file is part of x5250.
 *
 * x5250 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * x5250 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with x5250; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include "x5250.h"
#include "x5250term.h"
#include "resources.h"
#include "dialog.h"
#include "x5250icon.xbm"
#include "debug.h"
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <sys/types.h>
#include <unistd.h>



extern Display *display;
extern char *display_name;
extern int screen_num;
extern Screen *screen_ptr;
extern unsigned int ***buf5250;
extern unsigned int prevbuf[27][133];
extern screenfields *fields5250;
extern int subwindowcount;
extern int screencolumns;
extern int screenrows;
extern char *progname;
extern char *sessionname;
extern int placementx;
extern int placementy;
extern int mainwindowwidth;
extern int mainwindowheight;
extern char *font;
extern char *font132;
extern char *fontui;
extern int globalargc;
extern char **globalargv;
extern unsigned long foreground_pixel, background_pixel, border_pixel;
extern unsigned long menu_pixel, menuborder_pixel;
extern unsigned long red_pixel, green_pixel, blue_pixel, black_pixel,
  white_pixel, yellow_pixel, turquoise_pixel, pink_pixel, cursor_pixel;


/* Mapping of 5250 colors to curses colors */
#define A_5250_WHITE    0x100
#define A_5250_RED      0x200
#define A_5250_TURQ     0x300
#define A_5250_YELLOW   0x400
#define A_5250_PINK     0x500
#define A_5250_BLUE     0x600
#define A_5250_BLACK	0x700
#define A_5250_GREEN    0x800

#define A_COLOR_MASK	0xf00

#define A_REVERSE	0x1000
#define A_UNDERLINE	0x2000
#define A_BLINK		0x4000
#define A_VERTICAL	0x8000

static int attribute_map[] = {
  A_5250_GREEN,
  A_5250_GREEN | A_REVERSE,
  A_5250_WHITE,
  A_5250_WHITE | A_REVERSE,
  A_5250_GREEN | A_UNDERLINE,
  A_5250_GREEN | A_UNDERLINE | A_REVERSE,
  A_5250_WHITE | A_UNDERLINE,
  0x00,
  A_5250_RED,
  A_5250_RED | A_REVERSE,
  A_5250_RED | A_BLINK,
  A_5250_RED | A_BLINK | A_REVERSE,
  A_5250_RED | A_UNDERLINE,
  A_5250_RED | A_UNDERLINE | A_REVERSE,
  A_5250_RED | A_UNDERLINE | A_BLINK,
  0x00,
  A_5250_TURQ | A_VERTICAL,
  A_5250_TURQ | A_VERTICAL | A_REVERSE,
  A_5250_YELLOW | A_VERTICAL,
  A_5250_YELLOW | A_VERTICAL | A_REVERSE,
  A_5250_TURQ | A_UNDERLINE | A_VERTICAL,
  A_5250_TURQ | A_UNDERLINE | A_REVERSE | A_VERTICAL,
  A_5250_YELLOW | A_UNDERLINE | A_VERTICAL,
  0x00,
  A_5250_PINK,
  A_5250_PINK | A_REVERSE,
  A_5250_BLUE,
  A_5250_BLUE | A_REVERSE,
  A_5250_PINK | A_UNDERLINE,
  A_5250_PINK | A_UNDERLINE | A_REVERSE,
  A_5250_BLUE | A_UNDERLINE,
  0x00
};


#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))



static void x5250_terminal_init (Tn5250Terminal * terminal);
static int x5250_terminal_width (Tn5250Terminal * terminal);
static int x5250_terminal_height (Tn5250Terminal * terminal);
static int x5250_terminal_flags (Tn5250Terminal * terminal);
static void x5250_terminal_term (Tn5250Terminal * terminal);
static void x5250_terminal_destroy (Tn5250Terminal * terminal);
static void x5250_terminal_update (Tn5250Terminal * terminal,
				   Tn5250Display * display);
static void x5250_terminal_update_indicators (Tn5250Terminal * terminal,
					      Tn5250Display * display);
static int x5250_terminal_waitevent (Tn5250Terminal * terminal);
static int x5250_terminal_getkey (Tn5250Terminal * terminal);
static void x5250_terminal_putkey (Tn5250Terminal * terminal,
				   Tn5250Display * display, unsigned char key,
				   int row, int column);
static void x5250_terminal_beep (Tn5250Terminal * terminal);
#ifdef ENHANCED
static int x5250_terminal_enhanced (Tn5250Terminal * terminal);
static void x5250_terminal_create_window (Tn5250Terminal * terminal,
					  Tn5250Display * display,
					  Tn5250Window * window);
static void x5250_terminal_destroy_window (Tn5250Terminal * terminal,
					   Tn5250Display * display,
					   Tn5250Window * window);
static void x5250_terminal_create_scrollbar (Tn5250Terminal * terminal,
					     Tn5250Display * display,
					     Tn5250Scrollbar * scrollbar);
static void x5250_terminal_destroy_scrollbar (Tn5250Terminal * terminal,
					      Tn5250Display * display);
static void x5250_terminal_create_menubar (Tn5250Terminal * terminal,
					   Tn5250Display * display,
					   Tn5250Menubar * menubar);
static void x5250_terminal_destroy_menubar (Tn5250Terminal * terminal,
					    Tn5250Display * display,
					    Tn5250Menubar * menubar);
static void x5250_terminal_create_menuitem (Tn5250Terminal * terminal,
					    Tn5250Display * display,
					    Tn5250Menuitem * menuitem);
static void x5250_terminal_destroy_menuitem (Tn5250Terminal * terminal,
					     Tn5250Display * display,
					     Tn5250Menuitem * menuitem);
#endif



Tn5250Terminal *
x5250_terminal_new ()
{
  Tn5250Terminal *terminal = tn5250_new (Tn5250Terminal, 1);
  if (terminal == NULL)
    {
      return NULL;
    }

  terminal->data = tn5250_new (struct _Tn5250TerminalPrivate, 1);
  if (terminal->data == NULL)
    {
      free (terminal);
      return NULL;
    }

  terminal->conn_fd = -1;
  terminal->init = x5250_terminal_init;
  terminal->term = x5250_terminal_term;
  terminal->destroy = x5250_terminal_destroy;
  terminal->width = x5250_terminal_width;
  terminal->height = x5250_terminal_height;
  terminal->flags = x5250_terminal_flags;
  terminal->update = x5250_terminal_update;
  terminal->update_indicators = x5250_terminal_update_indicators;
  terminal->waitevent = x5250_terminal_waitevent;
  terminal->getkey = x5250_terminal_getkey;
  terminal->putkey = x5250_terminal_putkey;
  terminal->beep = x5250_terminal_beep;
  terminal->config = NULL;
#ifdef ENHANCED
  terminal->enhanced = x5250_terminal_enhanced;
  terminal->create_window = x5250_terminal_create_window;
  terminal->destroy_window = x5250_terminal_destroy_window;
  terminal->create_scrollbar = x5250_terminal_create_scrollbar;
  terminal->destroy_scrollbar = x5250_terminal_destroy_scrollbar;
  terminal->create_menubar = x5250_terminal_create_menubar;
  terminal->destroy_menubar = x5250_terminal_destroy_menubar;
  terminal->create_menuitem = x5250_terminal_create_menuitem;
  terminal->destroy_menuitem = x5250_terminal_destroy_menuitem;
#endif
  return terminal;
}




static void
x5250_terminal_init (Tn5250Terminal * terminal)
{
  int i;
  unsigned int display_width, display_height;
  unsigned int x, y;		/* window position */
  char *window_name;
  char window_text[72];
  char *icon_name;
  char icon_text[64];
  Pixmap icon_pixmap;
  XIconSize *size_list;
  XWMHints *wm_hints;
  XClassHint *class_hints;
  XTextProperty windowName, iconName;
  XSetWindowAttributes windowattribs;
  int count;
  double ratio;



  terminal->data->popup_menuitemscount = 8;
  terminal->data->fontsize = 0;
  terminal->data->font132size = 0;
  terminal->data->popped = False;
  terminal->data->borderwidth = 4;
  terminal->data->fieldid = 0;
  terminal->data->currentscreencolumns = 80;
  terminal->data->keypress = -1;
  terminal->data->refreshscreen = 0;
  terminal->data->pasterow = 0;
  terminal->data->pastecolumn = 0;
  terminal->data->selectbegrow = 0;
  terminal->data->selectbegcolumn = 0;
  terminal->data->selectendrow = 0;
  terminal->data->selectendcolumn = 0;
  terminal->data->ind_message = 0;
  terminal->data->ind_busy = 0;
  terminal->data->ind_lock = 0;
  terminal->data->ind_wait = 0;
  terminal->data->ind_insert = 0;
  terminal->data->alignpaste = False;
  terminal->data->pastecursor = False;
  terminal->data->ruler = False;
  terminal->data->scrollbar = False;
  terminal->data->havefocus = False;
  terminal->data->showattributes = False;
  terminal->data->columnseparators = False;
  memset (terminal->data->selection, '\0',
	  sizeof (terminal->data->selection));

  if (!(terminal->data->size_hints = XAllocSizeHints ()))
    {
      fprintf (stderr, "%s: failure allocating memory\n", progname);
      exit (0);
    }
  if (!(wm_hints = XAllocWMHints ()))
    {
      fprintf (stderr, "%s: failure allocating memory\n", progname);
      exit (0);
    }
  if (!(class_hints = XAllocClassHint ()))
    {
      fprintf (stderr, "%s: failure allocating memory\n", progname);
      exit (0);
    }

  terminal->data->size_hints->flags = PSize | PMinSize;



  /* connect to X server */

  if ((display = XOpenDisplay (display_name)) == NULL)
    {
      fprintf (stderr,
	       "%s: Can't open display '%s'\n", progname,
	       XDisplayName (display_name));
      exit (-1);
    }

  terminal->data->xconnectionfd = XConnectionNumber (display);


  /* Get the X resources */
  merge_db_X ();
  get_resources_X (terminal);


  /* Load the key maps */
  load_keymap ();


  /* get screen_num size from display structure macro */
  screen_num = DefaultScreen (display);
  screen_ptr = DefaultScreenOfDisplay (display);
  display_width = DisplayWidth (display, screen_num);
  display_height = DisplayHeight (display, screen_num);

  if (is_font_scalable (font) || is_font_scalable (fontui))
    {
      terminal->data->fontsize = 100;
    }

  if (is_font_scalable (font132))
    {
      terminal->data->font132size = 100;
    }

  /* We don't need to load the 132 column font yet because the sign on screen
   * is only 80 columns.  When the user views a 132 column screen we will
   * load the 132 column font.
   */
  load_font (&terminal->data->font_info, font, terminal->data->fontsize);
  load_font (&terminal->data->ui_font_info, fontui, terminal->data->fontsize);

  /* place window */
  if (placementx != 0)
    {
      terminal->data->size_hints->flags |= USPosition;
    }

  if (placementy != 0)
    {
      terminal->data->size_hints->flags |= USPosition;
    }

  terminal->data->size_hints->x = placementx;
  terminal->data->size_hints->y = placementy;

  /* size window with enough room for text */
  terminal->data->width =
    ((terminal->data->font_info->max_bounds.width) * screencolumns) +
    WINDOWMARGIN;
  terminal->data->size_hints->min_width = terminal->data->width;

  if (is_font_scalable (font))
    {
      if (mainwindowwidth > 0)
	{
	  /* for some reason we need to reduce the mainwindowwidth by 10%
	   * in order to get a font that fits inside the window.
	   */
	  ratio = ((double) mainwindowwidth * 0.9) / terminal->data->width;
	  terminal->data->fontsize = terminal->data->fontsize * ratio;
	  load_font (&terminal->data->font_info, font,
		     terminal->data->fontsize);
	  terminal->data->width = mainwindowwidth;
	}
    }
  else if (mainwindowwidth > terminal->data->width)
    {
      terminal->data->width = mainwindowwidth;
    }

  terminal->data->size_hints->width = terminal->data->width;

  /* the height need to be the maximum size of the font plus the spacing
   * between the rows times the number of rows plus the size of the indicator
   * window.
   */
  terminal->data->height =
    ((terminal->data->font_info->max_bounds.ascent +
      terminal->data->font_info->max_bounds.descent +
      ROWSPACING) * screenrows) + WINDOWMARGIN +
    terminal->data->ui_font_info->max_bounds.ascent +
    terminal->data->ui_font_info->max_bounds.descent +
    (terminal->data->borderwidth * 2);
  terminal->data->size_hints->min_height = terminal->data->height;

  if (mainwindowheight > terminal->data->height)
    {
      terminal->data->height = mainwindowheight;
    }

  terminal->data->size_hints->height = terminal->data->height;

  alloccolors ();



  /* Create the screen and field buffers and fill them with data from the
   * remote host (for now we just hardcode some screens).  We do this here
   * so that we have an appropriate window size to build the window with.
   */
  buf5250 = malloc (sizeof (unsigned int **));
  buf5250[0] = malloc (27 * sizeof (unsigned int *));
  for (i = 0; i < 27; i++)
    {
      buf5250[0][i] = malloc (133 * sizeof (unsigned int));
      memset (buf5250[0][i], 0, 133 * sizeof (unsigned int));
    }
  fields5250 = (screenfields *) malloc (sizeof (screenfields));
  subwindowcount = fillbuf (0, subwindowcount);



  /* create opaque window */
  terminal->data->win =
    XCreateSimpleWindow (display, RootWindow (display, screen_num),
			 placementx, placementy, terminal->data->width,
			 terminal->data->height, terminal->data->borderwidth,
			 border_pixel, background_pixel);

  /* Get available icon sizes from Window manager */

  if (XGetIconSizes (display, RootWindow (display, screen_num),
		     &size_list, &count) == 0)
    fprintf (stderr,
	     "%s: Window manager didn't set icon sizes - using default.\n",
	     progname);
  else
    {
      ;
      /* A real application would search through size_list
       * here to find an acceptable icon size, and then
       * create a pixmap of that size.  This requires
       * that the application have data for several sizes
       * of icons. */
    }

  /* Create pixmap of depth 1 (bitmap) for icon */
  icon_pixmap =
    XCreateBitmapFromData (display, terminal->data->win, x5250icon_bits,
			   x5250icon_width, x5250icon_height);

  /* These calls store window_name and icon_name into
   * XTextProperty structures and set their other 
   * fields properly. */
  memset (window_text, '\0', sizeof (window_text));
  sprintf (window_text, "%s", sessionname);
  window_name = window_text;
  if (XStringListToTextProperty (&window_name, 1, &windowName) == 0)
    {
      fprintf (stderr,
	       "%s: structure allocation for windowName failed.\n", progname);
      exit (-1);
    }

  memset (icon_text, '\0', sizeof (icon_text));
  sprintf (icon_text, "%s", sessionname);
  icon_name = icon_text;
  if (XStringListToTextProperty (&icon_name, 1, &iconName) == 0)
    {
      fprintf (stderr,
	       "%s: structure allocation for iconName failed.\n", progname);
      exit (-1);
    }

  wm_hints->initial_state = NormalState;
  wm_hints->input = True;
  wm_hints->icon_pixmap = icon_pixmap;
  wm_hints->flags = StateHint | IconPixmapHint | InputHint;

  class_hints->res_name = progname;
  class_hints->res_class = "x5250";

  XSetWMProperties (display, terminal->data->win, &windowName, &iconName,
		    globalargv, globalargc, terminal->data->size_hints,
		    wm_hints, class_hints);

  /* set Properties for window manager (always before mapping) */
  XSetStandardProperties (display, terminal->data->win, window_name,
			  icon_name, icon_pixmap, globalargv, globalargc,
			  terminal->data->size_hints);

  /* Select event types wanted */
  XSelectInput (display, terminal->data->win, ExposureMask | KeyPressMask |
		ButtonPressMask | ButtonReleaseMask | Button1MotionMask |
		StructureNotifyMask | FocusChangeMask);

  /* create GC for text and drawing */
  get_GC (terminal->data->win, &terminal->data->gcgreen,
	  terminal->data->font_info, 1);
  get_GC (terminal->data->win, &terminal->data->gcwhite,
	  terminal->data->font_info, 2);
  get_GC (terminal->data->win, &terminal->data->gcred,
	  terminal->data->font_info, 3);
  get_GC (terminal->data->win, &terminal->data->gcblue,
	  terminal->data->font_info, 4);
  get_GC (terminal->data->win, &terminal->data->gcyellow,
	  terminal->data->font_info, 5);
  get_GC (terminal->data->win, &terminal->data->gcturquoise,
	  terminal->data->font_info, 6);
  get_GC (terminal->data->win, &terminal->data->gcpink,
	  terminal->data->font_info, 7);
  get_GC (terminal->data->win, &terminal->data->gcblack,
	  terminal->data->font_info, 8);
  get_GC (terminal->data->win, &terminal->data->gcui,
	  terminal->data->ui_font_info, 9);
  get_GC (terminal->data->win, &terminal->data->gcmenu,
	  terminal->data->font_info, 10);

  /* Create a cursor for this window */
  terminal->data->cursor_xterm = XCreateFontCursor (display, XC_xterm);
  terminal->data->cursor_watch = XCreateFontCursor (display, XC_watch);
  terminal->data->cursor_x = XCreateFontCursor (display, XC_X_cursor);
  XDefineCursor (display, terminal->data->win, terminal->data->cursor_xterm);

  /* Display window */
  XMapWindow (display, terminal->data->win);



  y =
    terminal->data->height -
    (terminal->data->ui_font_info->max_bounds.ascent +
     terminal->data->ui_font_info->max_bounds.descent + WINDOWMARGIN);
  windowattribs.background_pixel = background_pixel;
  windowattribs.border_pixel = border_pixel;
  windowattribs.bit_gravity = WestGravity;
  windowattribs.win_gravity = SouthWestGravity;
  terminal->data->indwin =
    XCreateWindow (display, terminal->data->win, 0, y,
		   terminal->data->width - (terminal->data->borderwidth * 2),
		   (terminal->data->ui_font_info->max_bounds.ascent +
		    terminal->data->ui_font_info->max_bounds.descent),
		   terminal->data->borderwidth, CopyFromParent,
		   CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWBitGravity | CWWinGravity,
		   &windowattribs);
  XSelectInput (display, terminal->data->indwin,
		ExposureMask | ButtonPressMask);
  terminal->data->cursor_left = XCreateFontCursor (display, XC_left_ptr);
  XDefineCursor (display, terminal->data->indwin,
		 terminal->data->cursor_left);
  XMapWindow (display, terminal->data->indwin);


  createdialog (terminal);
  createdebugdialog (terminal);

  /* Define the enter key as a grab key.  Once enter is pressed the keyboard
   * will be grabbed and cooked key events will not be sent to us until
   * we tell the X server we are ready for them.
   */
  /*
     XGrabKey (display, XKeysymToKeycode (display, XK_Return), 0,
     terminal->data->win, True, GrabModeAsync, GrabModeSync);
   */


  terminal->data->row =
    fields5250[subwindowcount].fields[terminal->data->fieldid].row;
  terminal->data->column =
    fields5250[subwindowcount].fields[terminal->data->fieldid].column;
  return;
}




static void
x5250_terminal_term (Tn5250Terminal * terminal)
{
  XUndefineCursor (display, terminal->data->win);
  XUndefineCursor (display, terminal->data->indwin);
  XFreeCursor (display, terminal->data->cursor_xterm);
  XFreeCursor (display, terminal->data->cursor_watch);
  XFreeCursor (display, terminal->data->cursor_x);
  XFreeCursor (display, terminal->data->cursor_left);
  XUnloadFont (display, terminal->data->font_info->fid);
  XUnloadFont (display, terminal->data->ui_font_info->fid);
  XFreeGC (display, terminal->data->gcgreen);
  XFreeGC (display, terminal->data->gcwhite);
  XFreeGC (display, terminal->data->gcred);
  XFreeGC (display, terminal->data->gcblue);
  XFreeGC (display, terminal->data->gcyellow);
  XFreeGC (display, terminal->data->gcturquoise);
  XFreeGC (display, terminal->data->gcpink);
  XFreeGC (display, terminal->data->gcblack);
  XCloseDisplay (display);
  free (fields5250);
  free (buf5250);
  return;
}




static void
x5250_terminal_destroy (Tn5250Terminal * terminal)
{
  if (terminal->data != NULL)
    {
      free (terminal->data);
    }
  free (terminal);
  return;
}




static int
x5250_terminal_width (Tn5250Terminal * terminal)
{
  return screencolumns;
}




static int
x5250_terminal_height (Tn5250Terminal * terminal)
{
  return screenrows;
}




static int
x5250_terminal_flags (Tn5250Terminal * terminal)
{
  int f = 0;

  f |= TN5250_TERMINAL_HAS_COLOR;
  return f;
}




static void
x5250_terminal_update (Tn5250Terminal * terminal,
		       Tn5250Display * tn5250display)
{
  XEvent eventsend;
  Tn5250Field *iter;
  Tn5250Window *currentwin;
  int i, j;
  int attribute;
  unsigned char attr_idx = 0x20, c;
  unsigned int x5250attr;
  int fieldcount;
  int contlength;
  int windowrowoffset;
  int windowcolumnoffset;
  int startrow, startcolumn;
  int endrow, endcolumn;
  static int oldrow = 0;
  static int oldcolumn = 0;


  /*
     XAllowEvents (display, AsyncKeyboard, CurrentTime);
   */

#ifdef ENHANCED
  if (subwindowcount == 0)
    {
#endif
      windowcolumnoffset = 0;
      windowrowoffset = 0;
      currentwin = NULL;
#ifdef ENHANCED
    }
  else
    {
      currentwin =
	tn5250_window_list_find_by_id (tn5250display->display_buffers->
				       window_list, subwindowcount - 1);
      if (currentwin != NULL)
	{
	  windowrowoffset = tn5250_window_start_row (currentwin);
	  windowcolumnoffset = tn5250_window_start_col (currentwin) + 1;
	}
    }
#endif


  /* First we find out what character to draw at the current row and column.
   * We found out what key was pressed in key.c.  Here we draw the key as
   * lib5250 tells us to before incrementing the row/column position and
   * drawing the caret.  Only do this if we aren't going to redraw the screen
   * later.
   */
  if (terminal->data->refreshscreen == 0)
    {
      /* Only redraw if we are in a field.  If we aren't, there is no
       * reason to draw since they can't have changed anything.
       */
      if (is_in_field
	  (terminal->data->row - windowrowoffset,
	   terminal->data->column - windowcolumnoffset,
	   &terminal->data->fieldid))
	{
	  Tn5250Field *field = tn5250_display_current_field (tn5250display);

	  /* If the current field is word wrap, force an refresh */
	  if (((field != NULL) && tn5250_field_is_wordwrap (field))
	      || ((field != NULL) && tn5250_field_is_wordwrap (field->prev)))
	    {
	      terminal->data->refreshscreen = 1;
	    }
	}
    }

  terminal->data->row = tn5250_display_cursor_y (tn5250display);
  terminal->data->column = tn5250_display_cursor_x (tn5250display);

  if (subwindowcount == 0)
    {
      draw_caret (terminal->data->win,
		  terminal->data->gcyellow,
		  terminal->data->font_info,
		  terminal->data->row, terminal->data->column,
		  terminal->data->havefocus, terminal->data->ruler,
		  subwindowcount);
    }
  else
    {
      draw_caret (currentwin->data->win, terminal->data->gcyellow,
		  terminal->data->font_info,
		  terminal->data->row - windowrowoffset,
		  terminal->data->column - windowcolumnoffset,
		  terminal->data->havefocus, terminal->data->ruler,
		  subwindowcount);
    }

  if (tn5250display->display_buffers->scrollbar_count > 0)
    {
      draw_5250_scrollbar (terminal);
    }

  if (tn5250display->display_buffers->menubar_count > 0)
    {
      draw_5250_menubar (terminal);
    }

  x5250_terminal_update_indicators (terminal, tn5250display);

  if (terminal->data->refreshscreen == 0)
    {
      oldrow = terminal->data->row;
      oldcolumn = terminal->data->column;
      return;
    }

  terminal->data->refreshscreen = 0;

  if ((screencolumns != tn5250_display_width (tn5250display))
      || (screenrows != tn5250_display_height (tn5250display)))
    {
      screencolumns = tn5250_display_width (tn5250display);
      screenrows = tn5250_display_height (tn5250display);

      if (screencolumns == 132)
	{
	  load_font (&terminal->data->font_info, font132,
		     terminal->data->font132size);
	}
      else
	{
	  load_font (&terminal->data->font_info, font,
		     terminal->data->fontsize);
	}

      XSetFont (display, terminal->data->gcgreen,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcwhite,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcred,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcblue,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcyellow,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcturquoise,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcpink,
		terminal->data->font_info->fid);
      XSetFont (display, terminal->data->gcblack,
		terminal->data->font_info->fid);

      terminal->data->width =
	((terminal->data->font_info->max_bounds.width) * screencolumns) +
	WINDOWMARGIN;

      if (mainwindowwidth > terminal->data->width)
	{
	  terminal->data->width = mainwindowwidth;
	}

      terminal->data->height =
	((terminal->data->font_info->max_bounds.ascent +
	  terminal->data->font_info->max_bounds.descent +
	  ROWSPACING) * screenrows) + WINDOWMARGIN +
	terminal->data->ui_font_info->max_bounds.ascent +
	terminal->data->ui_font_info->max_bounds.descent +
	(terminal->data->borderwidth * 2);

      if (mainwindowheight > terminal->data->height)
	{
	  terminal->data->height = mainwindowheight;
	}

      terminal->data->size_hints->min_width = terminal->data->width;
      terminal->data->size_hints->min_height = terminal->data->height;
      XResizeWindow (display, terminal->data->win, terminal->data->width,
		     terminal->data->height);
      XResizeWindow (display, terminal->data->indwin,
		     terminal->data->width -
		     (terminal->data->borderwidth * 2),
		     (terminal->data->ui_font_info->max_bounds.ascent +
		      terminal->data->ui_font_info->max_bounds.descent));
      eventsend.type = Expose;
      if ((XSendEvent (display, terminal->data->win, False, 0, &eventsend)) ==
	  0)
	{
#ifdef DEBUG
	  fprintf (stderr, "x5250 (pid %d): XSendEvent failed!\n", getpid ());
#endif
	}
      else
	{
#ifdef DEBUG
	  fprintf
	    (stderr,
	     "x5250 (pid %d): sent expose event after resizing main window\n",
	     getpid ());
#endif
	}

      if (tn5250display->display_buffers->window_count > 0)
	{
	  Tn5250Window *iter, *next;

	  if ((iter = tn5250display->display_buffers->window_list) != NULL)
	    {
	      do
		{
		  next = iter->next;
		  XResizeWindow (display, iter->data->win,
				 iter->width *
				 terminal->data->font_info->max_bounds.width +
				 4,
				 ((terminal->data->font_info->max_bounds.
				   ascent +
				   terminal->data->font_info->max_bounds.
				   descent + ROWSPACING) * iter->height) + 4);
		  XSendEvent (display, iter->data->win, False, 0, &eventsend);
		  iter = next;
		}
	      while (iter != tn5250display->display_buffers->window_list);
	    }
	}
    }

  /* Initialize the screen and fields */
  destroy_screen (subwindowcount);
  fieldcount = 0;

  if ((iter = tn5250display->display_buffers->field_list) != NULL)
    {
      do
	{
	  if (tn5250_field_is_bypass (iter))
	    {
	    }
	  else
	    {
	      fields5250[subwindowcount].fields[fieldcount].fieldid =
		fieldcount;
	      fields5250[subwindowcount].fields[fieldcount].inputcapable = 1;
	      fields5250[subwindowcount].fields[fieldcount].continuous = 0;
	      fields5250[subwindowcount].fields[fieldcount].wordwrap = 0;
	      fields5250[subwindowcount].fields[fieldcount].row =
		iter->start_row - windowrowoffset;
	      fields5250[subwindowcount].fields[fieldcount].column =
		iter->start_col - windowcolumnoffset;
	      fields5250[subwindowcount].fields[fieldcount].length =
		iter->length;
	      fields5250[subwindowcount].
		fields[fieldcount].nextfieldprogressionid = fieldcount + 1;

	      contlength = 0;
	      while ((fields5250[subwindowcount].fields[fieldcount].column +
		      fields5250[subwindowcount].fields[fieldcount].length +
		      windowcolumnoffset) > screencolumns)
		{
		  fields5250[subwindowcount].fields[fieldcount].length =
		    screencolumns -
		    fields5250[subwindowcount].fields[fieldcount].column;
		  contlength +=
		    fields5250[subwindowcount].fields[fieldcount].length;
		  fields5250[subwindowcount].fields[fieldcount].continuous =
		    1;
		  fieldcount++;
		  fields5250[subwindowcount].fields[fieldcount].fieldid =
		    fieldcount;
		  fields5250[subwindowcount].fields[fieldcount].inputcapable =
		    1;
		  fields5250[subwindowcount].fields[fieldcount].continuous =
		    0;
		  fields5250[subwindowcount].fields[fieldcount].wordwrap = 0;
		  fields5250[subwindowcount].fields[fieldcount].row =
		    fields5250[subwindowcount].fields[fieldcount - 1].row + 1;
		  fields5250[subwindowcount].fields[fieldcount].column = 0;
		  fields5250[subwindowcount].fields[fieldcount].length =
		    iter->length - contlength;
		  fields5250[subwindowcount].
		    fields[fieldcount].nextfieldprogressionid =
		    fieldcount + 1;
		}
	      fieldcount++;
	    }
	  iter = iter->next;
	}
      while (iter != tn5250display->display_buffers->field_list);
    }
  fields5250[subwindowcount].totalfields = fieldcount;

  if (fieldcount > 0)
    {
      fieldcount--;
    }
  fields5250[subwindowcount].fields[fieldcount].nextfieldprogressionid = 0;

  startrow = 0;
  startcolumn = 0;
  endrow = screenrows;
  endcolumn = screencolumns;

#ifdef ENHANCED
  if (currentwin != NULL)
    {
      startrow = tn5250_window_start_row (currentwin);
      startcolumn = tn5250_window_start_col (currentwin) + 1;
      endrow = tn5250_window_start_row (currentwin) +
	tn5250_window_height (currentwin);
      endcolumn = startcolumn + tn5250_window_width (currentwin) + 1;
    }
#endif

  for (i = startrow; i < endrow; i++)
    {
      for (j = startcolumn; j < endcolumn; j++)
	{
	  c = tn5250_display_char_at (tn5250display, i, j);

	  if ((c & 0xe0) == 0x20)
	    {
	      attr_idx = (c & 0xff);
	      if (terminal->data->showattributes == False)
		{
		  buf5250[subwindowcount][i - windowrowoffset][j -
							       windowcolumnoffset]
		    = 0;
		}
	      else
		{
		  buf5250[subwindowcount][i - windowrowoffset][j -
							       windowcolumnoffset]
		    = '@' | A_X5250_PINK | A_X5250_NORMAL;
		}
	    }
	  else
	    {
	      attribute = attribute_map[attr_idx - 0x20];
	      x5250attr = 0;

	      if ((attribute & A_UNDERLINE) != 0)
		{
		  x5250attr = x5250attr | A_X5250_UNDERLINE;
		}
	      if ((attribute & A_VERTICAL) != 0)
		{
		  x5250attr = x5250attr | A_X5250_COLUMNSEP;
		}
	      if ((attribute & A_REVERSE) != 0)
		{
		  x5250attr = x5250attr | A_X5250_REVERSE;
		}
	      if (attribute == 0x00)
		{
		  x5250attr = x5250attr | A_X5250_NONDISPLAY;
		}

	      switch (attribute & A_COLOR_MASK)
		{
		case A_5250_GREEN:
		  x5250attr = x5250attr | A_X5250_GREEN | A_X5250_NORMAL;
		  break;
		case A_5250_WHITE:
		  x5250attr = x5250attr | A_X5250_WHITE | A_X5250_NORMAL;
		  break;
		case A_5250_RED:
		  x5250attr = x5250attr | A_X5250_RED | A_X5250_NORMAL;
		  break;
		case A_5250_PINK:
		  x5250attr = x5250attr | A_X5250_PINK | A_X5250_NORMAL;
		  break;
		case A_5250_BLUE:
		  x5250attr = x5250attr | A_X5250_BLUE | A_X5250_NORMAL;
		  break;
		case A_5250_YELLOW:
		  x5250attr = x5250attr | A_X5250_YELLOW | A_X5250_NORMAL;
		  break;
		case A_5250_TURQ:
		  x5250attr = x5250attr | A_X5250_TURQUOISE | A_X5250_NORMAL;
		  break;
		default:
		  x5250attr = x5250attr | A_X5250_GREEN | A_X5250_NORMAL;
		  break;
		}
	      /* UNPRINTABLE -- print block */
	      if ((c == 0x1f) || (c == 0x3F))
		{
		  buf5250[subwindowcount][i - windowrowoffset][j -
							       windowcolumnoffset]
		    = ' ' | x5250attr | A_X5250_REVERSE;
		}
	      /* UNPRINTABLE -- print blank */
	      else if ((c < 0x40 && c > 0x00) || c == 0xff)
		{
		  buf5250[subwindowcount][i - windowrowoffset][j -
							       windowcolumnoffset]
		    = ' ' | x5250attr;
		}
	      else
		{
		  c =
		    tn5250_char_map_to_local (tn5250_display_char_map
					      (tn5250display), c);

		  /* Don't put spaces where we don't need them.  This prevents
		   * us from redrawing every position on the screen - even
		   * when there is nothing to draw.  If we have a blank that
		   * isn't underlined or reverse image then don't put anything
		   * in the buffer.
		   */
		  /*
		     if ((c == ' ')
		     && ((x5250attr & A_X5250_UNDERLINEMASK) !=
		     A_X5250_UNDERLINE)
		     && ((x5250attr & A_X5250_REVERSEMASK) !=
		     A_X5250_REVERSE)
		     && ((x5250attr & A_X5250_COLUMNSEPMASK) !=
		     A_X5250_COLUMNSEP)
		     && ((x5250attr & A_X5250_NONDISPLAYMASK) !=
		     A_X5250_NONDISPLAY))
		   */
		  if ((c == ' ') && (x5250attr == 0))
		    {
		      buf5250[subwindowcount][i - windowrowoffset][j -
								   windowcolumnoffset]
			= 0;
		    }
		  else
		    {
		      buf5250[subwindowcount][i - windowrowoffset][j -
								   windowcolumnoffset]
			= c | x5250attr;
		    }
		}

	      if (is_in_field (i - windowrowoffset,
			       j - windowcolumnoffset, &fieldcount))
		{
		  fields5250[subwindowcount].fields[fieldcount].attributes =
		    x5250attr;
		  fields5250[subwindowcount].fields[fieldcount].contents[j -
									 windowcolumnoffset
									 -
									 fields5250
									 [subwindowcount].
									 fields
									 [fieldcount].
									 column]
		    =
		    buf5250[subwindowcount][i - windowrowoffset][j -
								 windowcolumnoffset];
		}
	    }
	}			/* for (int j ... */
    }				/* for (int i ... */

  eventsend.type = Expose;
  if (subwindowcount == 0)
    {
      /*
         XClearWindow (display, terminal->data->win);
         XSendEvent (display, terminal->data->win, False, 0, &eventsend);
       */
      place_text (terminal->data->win,
		  terminal->data->gcgreen,
		  terminal->data->gcwhite,
		  terminal->data->gcred,
		  terminal->data->gcblue,
		  terminal->data->gcyellow,
		  terminal->data->gcturquoise,
		  terminal->data->gcpink,
		  terminal->data->gcblack,
		  terminal->data->font_info,
		  terminal->data->width, terminal->data->height, 0);
      place_graphics (terminal, terminal->data->win, 0);
      erase_caret (terminal, terminal->data->win,
		   terminal->data->gcgreen,
		   terminal->data->gcwhite,
		   terminal->data->gcred,
		   terminal->data->gcblue,
		   terminal->data->gcyellow,
		   terminal->data->gcturquoise,
		   terminal->data->gcpink,
		   terminal->data->gcblack,
		   terminal->data->font_info,
		   oldrow, oldcolumn, terminal->data->ruler, subwindowcount);
      draw_caret (terminal->data->win,
		  terminal->data->gcyellow,
		  terminal->data->font_info,
		  terminal->data->row,
		  terminal->data->column,
		  terminal->data->havefocus,
		  terminal->data->ruler, subwindowcount);
    }
  else
    {
      /*
         XClearWindow (display, currentwin->data->win);
         XSendEvent (display, currentwin->data->win, False, 0, &eventsend);
       */
      place_text (currentwin->data->win,
		  terminal->data->gcgreen,
		  terminal->data->gcwhite,
		  terminal->data->gcred,
		  terminal->data->gcblue,
		  terminal->data->gcyellow,
		  terminal->data->gcturquoise,
		  terminal->data->gcpink,
		  terminal->data->gcblack,
		  terminal->data->font_info,
		  terminal->data->width,
		  terminal->data->height, subwindowcount);
      place_graphics (terminal, currentwin->data->win, subwindowcount);
      erase_caret (terminal,
		   currentwin->data->win,
		   terminal->data->gcgreen,
		   terminal->data->gcwhite,
		   terminal->data->gcred,
		   terminal->data->gcblue,
		   terminal->data->gcyellow,
		   terminal->data->gcturquoise,
		   terminal->data->gcpink,
		   terminal->data->gcblack,
		   terminal->data->font_info,
		   oldrow - windowrowoffset,
		   oldcolumn - windowcolumnoffset,
		   terminal->data->ruler, subwindowcount);
      draw_caret (currentwin->data->win,
		  terminal->data->gcyellow,
		  terminal->data->font_info,
		  terminal->data->row - windowrowoffset,
		  terminal->data->column - windowcolumnoffset,
		  terminal->data->havefocus,
		  terminal->data->ruler, subwindowcount);
    }
  oldrow = terminal->data->row;
  oldcolumn = terminal->data->column;
  return;
}





static void
x5250_terminal_update_indicators (Tn5250Terminal * terminal,
				  Tn5250Display * tn5250display)
{
  Tn5250Window *windowlist, *subwindow;
  int indicators;

  indicators = tn5250_display_indicators (tn5250display);
#ifdef DEBUG
  fprintf (stderr, "updating terminal indicators...\n");
#endif

  if ((indicators & TN5250_DISPLAY_IND_MESSAGE_WAITING) != 0)
    {
      terminal->data->ind_message = 1;
    }
  else
    {
      terminal->data->ind_message = 0;
    }
  if ((indicators & TN5250_DISPLAY_IND_INHIBIT) != 0)
    {
      if (subwindowcount == 0)
	{
	  XDefineCursor (display, terminal->data->win,
			 terminal->data->cursor_x);
	}
      else
	{
	  windowlist =
	    terminal->data->tn5250display->display_buffers->window_list;
	  subwindow =
	    tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
	  XDefineCursor (display, subwindow->data->win,
			 terminal->data->cursor_x);
	}
      terminal->data->ind_lock = 1;
    }
  else if ((indicators & TN5250_DISPLAY_IND_X_CLOCK) != 0)
    {
      if (subwindowcount == 0)
	{
	  XDefineCursor (display, terminal->data->win,
			 terminal->data->cursor_watch);
	}
      else
	{
	  windowlist =
	    terminal->data->tn5250display->display_buffers->window_list;
	  subwindow =
	    tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
	  XDefineCursor (display, subwindow->data->win,
			 terminal->data->cursor_watch);
	}
      terminal->data->ind_wait = 1;
    }
  else if ((indicators & TN5250_DISPLAY_IND_X_SYSTEM) != 0)
    {
      if (subwindowcount == 0)
	{
	  XDefineCursor (display, terminal->data->win,
			 terminal->data->cursor_watch);
	}
      else
	{
	  windowlist =
	    terminal->data->tn5250display->display_buffers->window_list;
	  subwindow =
	    tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
	  XDefineCursor (display, subwindow->data->win,
			 terminal->data->cursor_watch);
	}
      terminal->data->ind_busy = 1;
    }
  else
    {
      XDefineCursor (display, terminal->data->win,
		     terminal->data->cursor_xterm);
      terminal->data->ind_lock = 0;
      terminal->data->ind_wait = 0;
      terminal->data->ind_busy = 0;
    }
  if ((indicators & TN5250_DISPLAY_IND_INSERT) != 0)
    {
      terminal->data->ind_insert = 1;
    }
  else
    {
      terminal->data->ind_insert = 0;
    }
  draw_indicator_win (terminal);
  return;
}





static int
x5250_terminal_waitevent (Tn5250Terminal * terminal)
{
  Tn5250Window *windowlist, *subwindow;
  KeySym keysym;
  XComposeStatus compose;
  XEvent report;
  XEvent eventsend;
  int window_size = 0;		/* OK, or too SMALL to display contents */
  int queuedevents = 0;
  fd_set filedescs;
  int checkselect;
  int maxfd = 0;
  char buffer[10] = { '\0' };
  int i;
  int result = 0;
  int button;
  int windowrowoffset;
  int windowcolumnoffset;
  double ratio;


#ifdef ENHANCED
  if (subwindowcount == 0)
    {
#endif
      windowcolumnoffset = 0;
      windowrowoffset = 0;
#ifdef ENHANCED
    }
  else
    {
      windowlist =
	terminal->data->tn5250display->display_buffers->window_list;
      subwindow =
	tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);

      /* For some reason we need to shift the column offset by an
       * additional column.
       */
      windowcolumnoffset = subwindow->column + 1;
      windowrowoffset = subwindow->row;
    }
#endif

#ifdef DEBUGEVENTS
  fprintf (stderr, "x5250 (pid %d): checking events...\n", getpid ());
#endif
  maxfd = max (maxfd, terminal->conn_fd);
  maxfd = max (maxfd, terminal->data->xconnectionfd);
  FD_ZERO (&filedescs);
  queuedevents = XPending (display);

  if (queuedevents < 1)
    {
      if (terminal->conn_fd >= 0)
	{
	  FD_SET (terminal->conn_fd, &filedescs);
	}
      FD_SET (terminal->data->xconnectionfd, &filedescs);
      checkselect = select (maxfd + 1, &filedescs, NULL, NULL, NULL);

      if (checkselect == -1)
	{
	  perror ("select()");
	}
      else if (checkselect == 0)
	{
	  fprintf (stderr, "Send a keep alive packet.\n");
	}
      else
	{
	  if (FD_ISSET (terminal->conn_fd, &filedescs))
	    {
#ifdef DEBUGEVENTS
	      fprintf (stderr, "x5250 (pid %d): found 5250 data to read\n",
		       getpid ());
#endif
	      terminal->data->refreshscreen = 1;
	      result |= TN5250_TERMINAL_EVENT_DATA;
#ifdef DEBUGEVENTS
	      fprintf (stderr, "x5250 (pid %d): done checking events...\n",
		       getpid ());
#endif
	      return result;
	    }
	}
    }
  if ((queuedevents > 0)
      || (FD_ISSET (terminal->data->xconnectionfd, &filedescs)))
    {
#ifdef DEBUGEVENTS
      fprintf (stderr, "x5250 (pid %d): found X event ", getpid ());
      if (queuedevents > 0)
	{
	  fprintf (stderr, "from queue\n");
	  fprintf (stderr, "x5250 (pid %d): found %d queued events\n",
		   getpid (), queuedevents);
	}
      else
	{
	  fprintf (stderr, "from X connection\n");
	}
#endif
      /* get events, use first to display text and graphics */
      while (XPending (display) != 0)
	{
	  XNextEvent (display, &report);

	  switch (report.type)
	    {
	    case SelectionNotify:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking selectionnotify event...\n",
		       getpid ());
#endif
	      if (report.xselection.property == None)
		{
		  printf ("Paste could not be completed.\n");
		}
	      else
		{
		  if (subwindowcount == 0)
		    {
		      paste_text (terminal->data->win,
				  report.xselection.property, terminal);
		    }
		  else
		    {
		      windowlist =
			terminal->data->tn5250display->display_buffers->
			window_list;
		      subwindow =
			tn5250_window_list_find_by_id (windowlist,
						       subwindowcount - 1);
		      paste_text (subwindow->data->win,
				  report.xselection.property, terminal);
		    }
		}
	      break;

	    case SelectionRequest:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking selectionrequest event...\n",
		       getpid ());
#endif
	      selectionhandler (&report, terminal);
	      break;

	    case SelectionClear:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking selectionclear event...\n",
		       getpid ());
#endif
	      /*
	         XSetSelectionOwner (display, report.xselectionclear.selection,
	         None, CurrentTime);
	       */
	      place_text (terminal->data->win,
			  terminal->data->gcgreen,
			  terminal->data->gcwhite,
			  terminal->data->gcred,
			  terminal->data->gcblue,
			  terminal->data->gcyellow,
			  terminal->data->gcturquoise,
			  terminal->data->gcpink,
			  terminal->data->gcblack,
			  terminal->data->font_info,
			  terminal->data->width, terminal->data->height, 0);
	      if (subwindowcount > 0)
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;

		  for (i = 1; i <= subwindowcount; i++)
		    {
		      subwindow =
			tn5250_window_list_find_by_id (windowlist, i - 1);
		      /* place text in window */
		      place_text (subwindow->data->win,
				  terminal->data->gcgreen,
				  terminal->data->gcwhite,
				  terminal->data->gcred,
				  terminal->data->gcblue,
				  terminal->data->gcyellow,
				  terminal->data->gcturquoise,
				  terminal->data->gcpink,
				  terminal->data->gcblack,
				  terminal->data->font_info,
				  terminal->data->width,
				  terminal->data->height, i);
		    }
		}
	      break;
	    case Expose:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking expose event...\n",
		       getpid ());
#endif
	      if (report.xexpose.window == terminal->data->popup)
		{
		  drawdialog (terminal);
		  break;
		}

	      for (i = 0; i < terminal->data->popup_menuitemscount; i++)
		{
		  if (report.xexpose.window ==
		      terminal->data->popup_menuitems[i])
		    {
		      drawdialog (terminal);
		      break;
		    }
		}

	      if ((report.xexpose.window == terminal->data->debugpopup)
		  || (report.xexpose.window ==
		      terminal->data->debugpopup_menuitem1)
		  || (report.xexpose.window ==
		      terminal->data->debugpopup_menuitem2))
		{
		  drawdebugdialog (terminal);
		  break;
		}

	      if (report.xexpose.window == terminal->data->macropopup)
		{
		  drawmacrodialog (terminal,
				   terminal->data->tn5250display->macro);
		  break;
		}

	      for (i = 0; i < terminal->data->macropopup_menuitemscount; i++)
		{
		  if (report.xexpose.window ==
		      terminal->data->macropopup_menuitems[i])
		    {
		      drawmacrodialog (terminal,
				       terminal->data->tn5250display->macro);
		      break;
		    }
		}

	      /* get all other Expose events on the queue */
	      while (XCheckTypedEvent (display, Expose, &report))
		{
		  if (report.xexpose.window == terminal->data->popup)
		    {
		      drawdialog (terminal);
		    }

		  for (i = 0; i < terminal->data->popup_menuitemscount; i++)
		    {
		      if (report.xexpose.window ==
			  terminal->data->popup_menuitems[i])
			{
			  drawdialog (terminal);
			}
		    }

		  if ((report.xexpose.window ==
		       terminal->data->debugpopup)
		      || (report.xexpose.window ==
			  terminal->data->debugpopup_menuitem1)
		      || (report.xexpose.window ==
			  terminal->data->debugpopup_menuitem2))
		    {
		      drawdebugdialog (terminal);
		    }

		  if (report.xexpose.window == terminal->data->macropopup)
		    {
		      drawmacrodialog (terminal,
				       terminal->data->tn5250display->macro);
		    }

		  for (i = 0; i < terminal->data->macropopup_menuitemscount;
		       i++)
		    {
		      if (report.xexpose.window ==
			  terminal->data->macropopup_menuitems[i])
			{
			  drawmacrodialog (terminal,
					   terminal->data->tn5250display->
					   macro);
			}
		    }
		}
	      /* place text in window */
	      place_text (terminal->data->win,
			  terminal->data->gcgreen,
			  terminal->data->gcwhite,
			  terminal->data->gcred,
			  terminal->data->gcblue,
			  terminal->data->gcyellow,
			  terminal->data->gcturquoise,
			  terminal->data->gcpink,
			  terminal->data->gcblack,
			  terminal->data->font_info,
			  terminal->data->width, terminal->data->height, 0);

	      /* place graphics in window */
	      place_graphics (terminal, terminal->data->win, 0);

	      if (subwindowcount > 0)
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;

		  for (i = 1; i <= subwindowcount; i++)
		    {
		      subwindow =
			tn5250_window_list_find_by_id (windowlist, i - 1);
		      /* place text in window */
		      place_text (subwindow->data->win,
				  terminal->data->gcgreen,
				  terminal->data->gcwhite,
				  terminal->data->gcred,
				  terminal->data->gcblue,
				  terminal->data->gcyellow,
				  terminal->data->gcturquoise,
				  terminal->data->gcpink,
				  terminal->data->gcblack,
				  terminal->data->font_info,
				  terminal->data->width,
				  terminal->data->height, i);

		      /* place graphics in window */
		      place_graphics (terminal, subwindow->data->win, i);
		    }
		}
	      if (subwindowcount == 0)
		{
		  draw_caret (terminal->data->win,
			      terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row,
			      terminal->data->column,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      else
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;
		  subwindow =
		    tn5250_window_list_find_by_id (windowlist,
						   subwindowcount - 1);
		  draw_caret (subwindow->data->win, terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row - windowrowoffset,
			      terminal->data->column - windowcolumnoffset,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      draw_indicator_win (terminal);

	      /* Draw any scrollbars */
	      if (terminal->data->tn5250display->display_buffers->
		  scrollbar_count > 0)
		{
		  draw_5250_scrollbar (terminal);
		}

	      /* Draw any menus */
	      if (terminal->data->tn5250display->display_buffers->
		  menubar_count > 0)
		{
		  draw_5250_menubar (terminal);
		}
	      break;
	    case ConfigureNotify:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking configurenotify event...\n",
		       getpid ());
#endif
	      if (report.xexpose.window == terminal->data->popup)
		{
		  break;
		}
	      /* window has been resized, change width and
	       * height to send to place_text and place_graphics
	       * in next Expose */

	      /* First figure out by what ratio the size has changed
	       * so that we can scale the font size if a scalabe font
	       * is in use.
	       */

	      if (terminal->data->width != 0)
		{
		  ratio =
		    ((double) report.xconfigure.width) /
		    terminal->data->width;
		  if (screencolumns == 132)
		    {
		      terminal->data->font132size =
			terminal->data->font132size * ratio;
		      load_font (&terminal->data->font_info, font132,
				 terminal->data->font132size);
		    }
		  else
		    {
		      terminal->data->fontsize =
			terminal->data->fontsize * ratio;
		      load_font (&terminal->data->font_info, font,
				 terminal->data->fontsize);
		    }

		  XSetFont (display, terminal->data->gcgreen,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcwhite,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcred,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcblue,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcyellow,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcturquoise,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcpink,
			    terminal->data->font_info->fid);
		  XSetFont (display, terminal->data->gcblack,
			    terminal->data->font_info->fid);
		}

	      terminal->data->width = report.xconfigure.width;
	      terminal->data->height =
		((terminal->data->font_info->max_bounds.ascent +
		  terminal->data->font_info->max_bounds.descent +
		  ROWSPACING) * screenrows) + WINDOWMARGIN +
		terminal->data->ui_font_info->max_bounds.ascent +
		terminal->data->ui_font_info->max_bounds.descent +
		(terminal->data->borderwidth * 2);

	      if (report.xconfigure.height < terminal->data->height)
		{
		  XResizeWindow (display, terminal->data->win,
				 terminal->data->width,
				 terminal->data->height);
		}

	      if ((terminal->data->width <
		   terminal->data->size_hints->min_width)
		  || (terminal->data->height <
		      terminal->data->size_hints->min_height))
		window_size = SMALL;
	      else
		{
		  window_size = OK;
		  XResizeWindow (display, terminal->data->indwin,
				 terminal->data->width -
				 (terminal->data->borderwidth * 2),
				 (terminal->data->ui_font_info->max_bounds.
				  ascent +
				  terminal->data->ui_font_info->max_bounds.
				  descent));
		  XClearWindow (display, terminal->data->win);
		  eventsend.type = Expose;
		  if ((XSendEvent (display, report.xexpose.window, False,
				   0, &eventsend)) == 0)
		    {
#ifdef DEBUGEVENTS
		      fprintf (stderr,
			       "x5250 (pid %d): XSendEvent failed!\n",
			       getpid ());
#endif
		    }
		  else
		    {
#ifdef DEBUGEVENTS
		      fprintf
			(stderr,
			 "x5250 (pid %d): sent expose event after checking configurenotify event\n",
			 getpid ());
#endif
		    }
		}
	      break;

	    case ButtonPress:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking button press event...\n",
		       getpid ());
#endif
	      button = buttonpresshandler (terminal, &report);
	      if (button == 1)
		{
		  if (terminal->data->keypress != -1)
		    {
		      result |= TN5250_TERMINAL_EVENT_KEY;
#ifdef DEBUGEVENTS
		      fprintf (stderr,
			       "x5250 (pid %d): done checking events...\n",
			       getpid ());
#endif
		      return result;
		    }
		}
	      else if (button == 3)
		{
		}
	      /* Button 0 means exit */
	      else if (button == 0)
		{
		  x5250_terminal_term (terminal);
		  x5250_terminal_destroy (terminal);
		  exit (0);
		}
	      break;

	    case ButtonRelease:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking button release event...\n",
		       getpid ());
#endif
	      button = buttonreleasehandler (terminal, &report);
#ifdef DEBUGEVENTS
	      if (button == 1)
		{
		  printf ("selection coords: %d %d : %d %d\n",
			  terminal->data->selectbegcolumn,
			  terminal->data->selectbegrow,
			  terminal->data->selectendcolumn,
			  terminal->data->selectendrow);
		}
#endif
	      break;

	    case MotionNotify:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking motion notify event...\n",
		       getpid ());
#endif
	      buttonmotionhandler (terminal, &report);
	      break;

	    case FocusIn:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking focus in event...\n",
		       getpid ());
#endif
	      terminal->data->havefocus = True;
	      /*
	         XAllowEvents (display, AsyncKeyboard, CurrentTime);
	       */

	      if (subwindowcount == 0)
		{
		  draw_caret (terminal->data->win,
			      terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row, terminal->data->column,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      else
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;
		  subwindow =
		    tn5250_window_list_find_by_id (windowlist,
						   subwindowcount - 1);
		  draw_caret (subwindow->data->win, terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row - windowrowoffset,
			      terminal->data->column - windowcolumnoffset,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      break;

	    case FocusOut:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking focus out event...\n",
		       getpid ());
#endif
	      /*
	         XAllowEvents (display, AsyncKeyboard, CurrentTime);
	         if (terminal->data->havefocus == True)
	         {
	         XUngrabKeyboard (display, CurrentTime);
	         }
	       */
	      terminal->data->havefocus = False;

	      if (subwindowcount == 0)
		{
		  erase_caret (terminal, terminal->data->win,
			       terminal->data->gcgreen,
			       terminal->data->gcwhite,
			       terminal->data->gcred,
			       terminal->data->gcblue,
			       terminal->data->gcyellow,
			       terminal->data->gcturquoise,
			       terminal->data->gcpink,
			       terminal->data->gcblack,
			       terminal->data->font_info,
			       terminal->data->row,
			       terminal->data->column,
			       terminal->data->ruler, subwindowcount);
		  draw_caret (terminal->data->win,
			      terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row, terminal->data->column,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      else
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;
		  subwindow =
		    tn5250_window_list_find_by_id (windowlist,
						   subwindowcount - 1);
		  erase_caret (terminal, subwindow->data->win,
			       terminal->data->gcgreen,
			       terminal->data->gcwhite, terminal->data->gcred,
			       terminal->data->gcblue,
			       terminal->data->gcyellow,
			       terminal->data->gcturquoise,
			       terminal->data->gcpink,
			       terminal->data->gcblack,
			       terminal->data->font_info,
			       terminal->data->row - windowrowoffset,
			       terminal->data->column - windowcolumnoffset,
			       terminal->data->ruler, subwindowcount);
		  draw_caret (subwindow->data->win, terminal->data->gcyellow,
			      terminal->data->font_info,
			      terminal->data->row - windowrowoffset,
			      terminal->data->column - windowcolumnoffset,
			      terminal->data->havefocus,
			      terminal->data->ruler, subwindowcount);
		}
	      break;

	    case KeyPress:
#ifdef DEBUGEVENTS
	      fprintf (stderr, "x5250 (pid %d): checking key event...\n",
		       getpid ());
#endif
	      terminal->data->keypress =
		keyhandler (&terminal->data->fieldid,
			    &terminal->data->row, &terminal->data->column,
			    &report, &keysym, &compose, buffer,
			    windowrowoffset, windowcolumnoffset);

	      /* keypress == -2 means exit */
	      if (terminal->data->keypress == -2)
		{
		  x5250_terminal_term (terminal);
		  x5250_terminal_destroy (terminal);
		  exit (0);
		}
	      /* keypress != -1 means the cursor is in an input field */
	      if (terminal->data->keypress != -1)
		{
		  if (subwindowcount == 0)
		    {
		      erase_caret (terminal, terminal->data->win,
				   terminal->data->gcgreen,
				   terminal->data->gcwhite,
				   terminal->data->gcred,
				   terminal->data->gcblue,
				   terminal->data->gcyellow,
				   terminal->data->gcturquoise,
				   terminal->data->gcpink,
				   terminal->data->gcblack,
				   terminal->data->font_info,
				   terminal->data->row,
				   terminal->data->column,
				   terminal->data->ruler, subwindowcount);
		    }
		  else
		    {
		      windowlist =
			terminal->data->tn5250display->display_buffers->
			window_list;
		      subwindow =
			tn5250_window_list_find_by_id (windowlist,
						       subwindowcount - 1);
		      erase_caret (terminal, subwindow->data->win,
				   terminal->data->gcgreen,
				   terminal->data->gcwhite,
				   terminal->data->gcred,
				   terminal->data->gcblue,
				   terminal->data->gcyellow,
				   terminal->data->gcturquoise,
				   terminal->data->gcpink,
				   terminal->data->gcblack,
				   terminal->data->font_info,
				   terminal->data->row - windowrowoffset,
				   terminal->data->column -
				   windowcolumnoffset, terminal->data->ruler,
				   subwindowcount);
		    }
		  /*
		     XGrabKeyboard (display, terminal->data->win, True,
		     GrabModeAsync, GrabModeSync,
		     CurrentTime);
		   */

		  if ((terminal->data->keypress == K_FIELDEXIT)
		      || (terminal->data->keypress == K_FIELDMINUS)
		      || (terminal->data->keypress == K_FIELDPLUS)
		      || (terminal->data->keypress == K_DELETE)
		      || (terminal->data->keypress == K_RESET)
		      || (terminal->data->ind_insert)
		      ||
		      ((terminal->data->tn5250display->destructive_backspace)
		       && (terminal->data->keypress == K_BACKSPACE)))
		    {
		      terminal->data->refreshscreen = 1;
		    }
		  result |= TN5250_TERMINAL_EVENT_KEY;
#ifdef DEBUGEVENTS
		  fprintf (stderr,
			   "x5250 (pid %d): done checking events...\n",
			   getpid ());
#endif
		  return result;
		}
	      break;

	    case MappingNotify:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking mappingnotify event...\n",
		       getpid ());
#endif
	      XRefreshKeyboardMapping (&report);
	      break;
	    default:
#ifdef DEBUGEVENTS
	      fprintf (stderr,
		       "x5250 (pid %d): checking unmonitored event...\n",
		       getpid ());
#endif
	      /* all events selected by StructureNotifyMask
	       * except ConfigureNotify are thrown away here,
	       * since nothing is done with them */
	      break;
	    }			/* end switch */
	}
    }
#ifdef DEBUGEVENTS
  fprintf (stderr, "x5250 (pid %d): done checking events...\n", getpid ());
#endif
  return result;
}





static int
x5250_terminal_getkey (Tn5250Terminal * terminal)
{
  int key;

#ifdef DEBUG
  fprintf (stderr, "sending key...\n");
#endif
  key = terminal->data->keypress;
  terminal->data->keypress = -1;
  return key;
}





static void
x5250_terminal_beep (Tn5250Terminal * terminal)
{
#ifdef DEBUG
  fprintf (stderr, "terminal beeping...\n");
#endif
  XBell (display, terminal->data->bellvolume);
  return;
}




#ifdef ENHANCED
static int
x5250_terminal_enhanced (Tn5250Terminal * terminal)
{
  return (1);
}




static void
x5250_terminal_create_window (Tn5250Terminal * terminal,
			      Tn5250Display * tn5250display,
			      Tn5250Window * window)
{
  unsigned int pop_width, pop_height;
  unsigned int x, y;		/* window size and position */
  int i, j;
  static int depth = 0;

#ifdef DEBUG
  printf ("creating a window\n");

  printf ("window width, height : x, y: %d, %d : %d, %d\n",
	  window->width, window->height, window->column, window->row);
#endif
  /*
     destroy_screen (subwindowcount);
   */

  window->data = tn5250_new (struct _Tn5250WindowPrivate, 1);

  if (window->data == NULL)
    {
      return;
    }

  /* Force the prevbuf to all 1's which will force the drawing routines to
   * redraw every position on the screen.  This is good since the new window
   * will most likely be offset from the previous screen and using it to
   * make drawing decisions can lead to things not being erased that should.
   */
  for (i = 0; i < 27; i++)
    {
      for (j = 0; j < 133; j++)
	{
	  prevbuf[i][j] = 1;
	}
    }

  /* For some reason, the size is supposed to be 2 greater than what the
   * iSeries sends us.
   */
  pop_width =
    (window->width + 2) * terminal->data->font_info->max_bounds.width + 4;
  pop_height =
    ((terminal->data->font_info->max_bounds.ascent +
      terminal->data->font_info->max_bounds.descent +
      ROWSPACING) * window->height) + 4;
  x = terminal->data->font_info->max_bounds.width * window->column;
  y =
    (terminal->data->font_info->max_bounds.ascent +
     terminal->data->font_info->max_bounds.descent +
     ROWSPACING) * window->row;
  window->data->win =
    XCreateSimpleWindow (display, terminal->data->win, x, y, pop_width,
			 pop_height, terminal->data->borderwidth,
			 border_pixel, background_pixel);
  XSelectInput (display, window->data->win,
		ExposureMask | KeyPressMask | ButtonPressMask |
		ButtonMotionMask);
  XMapWindow (display, window->data->win);
  subwindowcount++;

  if (subwindowcount > depth)
    {
      depth = subwindowcount;
      buf5250 = realloc (buf5250, (depth + 1) * sizeof (unsigned int **));
      buf5250[depth] = malloc (27 * sizeof (unsigned int *));
      for (i = 0; i < 27; i++)
	{
	  buf5250[depth][i] = malloc (133 * sizeof (unsigned int));
	  memset (buf5250[depth][i], 0, 133 * sizeof (unsigned int));
	}
      fields5250 = (screenfields *) realloc ((void *) fields5250,
					     (depth +
					      1) * sizeof (screenfields));
    }
  return;
}




static void
x5250_terminal_destroy_window (Tn5250Terminal * terminal,
			       Tn5250Display * tn5250display,
			       Tn5250Window * window)
{
  int i, j;

#ifdef DEBUG
  printf ("destroying a window\n");
#endif

  /* Force the prevbuf to all 1's which will force the drawing routines to
   * redraw every position on the screen.  This is good since the old window
   * was most likely offset from the current screen and using it to
   * make drawing decisions can lead to things not being erased that should.
   */
  for (i = 0; i < 27; i++)
    {
      for (j = 0; j < 133; j++)
	{
	  prevbuf[i][j] = 1;
	}
    }

  XDestroyWindow (display, window->data->win);
  free (window->data);

  if (subwindowcount > 0)
    {
      subwindowcount--;
    }
  return;
}




static void
x5250_terminal_create_scrollbar (Tn5250Terminal * terminal,
				 Tn5250Display * tn5250display,
				 Tn5250Scrollbar * scrollbar)
{
  terminal->data->scrollbar = True;
  return;
}




static void
x5250_terminal_destroy_scrollbar (Tn5250Terminal * terminal,
				  Tn5250Display * tn5250display)
{
  terminal->data->scrollbar = False;
  return;
}




static void
x5250_terminal_create_menubar (Tn5250Terminal * terminal,
			       Tn5250Display * tn5250display,
			       Tn5250Menubar * menubar)
{
  unsigned int width, height;
  unsigned int x, y;		/* window size and position */

  menubar->data = tn5250_new (struct _Tn5250MenubarPrivate, 1);

  if (menubar->data == NULL)
    {
      return;
    }

  width =
    ((tn5250_menubar_itemsize (menubar) + 2) *
     tn5250_menubar_items (menubar)) *
    terminal->data->font_info->max_bounds.width + 4;
  height =
    (terminal->data->font_info->max_bounds.ascent +
     terminal->data->font_info->max_bounds.descent +
     ROWSPACING + 4) * tn5250_menubar_height (menubar);
  x =
    terminal->data->font_info->max_bounds.width *
    (tn5250_menubar_start_col (menubar) - 1);
  y =
    (terminal->data->font_info->max_bounds.ascent +
     terminal->data->font_info->max_bounds.descent +
     ROWSPACING) * (tn5250_menubar_start_row (menubar));
  menubar->data->win =
    XCreateSimpleWindow (display, terminal->data->win, x, y, width, height, 1,
			 menuborder_pixel, menu_pixel);
  XSelectInput (display, menubar->data->win,
		ExposureMask | KeyPressMask | ButtonPressMask);
  XMapWindow (display, menubar->data->win);
  return;
}




static void
x5250_terminal_destroy_menubar (Tn5250Terminal * terminal,
				Tn5250Display * tn5250display,
				Tn5250Menubar * menubar)
{
  XDestroyWindow (display, menubar->data->win);
  free (menubar->data);
  return;
}




static void
x5250_terminal_create_menuitem (Tn5250Terminal * terminal,
				Tn5250Display * tn5250display,
				Tn5250Menuitem * menuitem)
{
  unsigned int width, height;
  unsigned int x, y;		/* window size and position */

  menuitem->data = tn5250_new (struct _Tn5250MenuitemPrivate, 1);

  if (menuitem->data == NULL)
    {
      return;
    }

  width =
    (tn5250_menubar_itemsize (menuitem->menubar) + 2) *
    terminal->data->font_info->max_bounds.width + 4;
  height = terminal->data->font_info->max_bounds.ascent +
    terminal->data->font_info->max_bounds.descent + ROWSPACING + 4;

  if (tn5250_menubar_type (menuitem->menubar) == MENU_TYPE_MENUBAR)
    {
      x = width * menuitem->id;
      y = 0;
    }
  else
    {
      x = 0;
      y = height * menuitem->id;
    }

  menuitem->data->win =
    XCreateSimpleWindow (display, menuitem->menubar->data->win, x, y, width,
			 height, 1, menuborder_pixel, menu_pixel);
  XSelectInput (display, menuitem->data->win,
		ExposureMask | KeyPressMask | ButtonPressMask);
  XMapWindow (display, menuitem->data->win);
  return;
}




static void
x5250_terminal_destroy_menuitem (Tn5250Terminal * terminal,
				 Tn5250Display * tn5250display,
				 Tn5250Menuitem * menuitem)
{
  XDestroyWindow (display, menuitem->data->win);
  free (menuitem->data);
  return;
}
#endif



static void
x5250_terminal_putkey (Tn5250Terminal * terminal,
		       Tn5250Display * tn5250display, unsigned char key,
		       int row, int column)
{
  unsigned char c;
  int windowrowoffset;
  int windowcolumnoffset;
  Tn5250Window *currentwin;

#ifdef ENHANCED
  if (subwindowcount == 0)
    {
#endif
      windowcolumnoffset = 0;
      windowrowoffset = 0;
      currentwin = NULL;
#ifdef ENHANCED
    }
  else
    {
      currentwin =
	tn5250_window_list_find_by_id (tn5250display->display_buffers->
				       window_list, subwindowcount - 1);
      if (currentwin != NULL)
	{
	  windowrowoffset = tn5250_window_start_row (currentwin);
	  windowcolumnoffset = tn5250_window_start_col (currentwin) + 1;
	}
    }
#endif

  /* For some reason terminal->data->row does not contain the right values
   * when pasting on some screens.  So ignore terminal->data->row and just
   * row all the time.
   * FIXME
   */
  /*
     if (terminal->data->row != row)
     {
     printf ("row != terminal->data->row! (%d %d)\n",
     row, terminal->data->row);
     }
     if (terminal->data->column != column)
     {
     printf ("column != terminal->data->column! (%d %d)\n",
     column, terminal->data->column);
     }

     if (is_in_field (terminal->data->row - windowrowoffset,
     terminal->data->column - windowcolumnoffset,
     &terminal->data->fieldid))
   */
  if (is_in_field (row - windowrowoffset,
		   column - windowcolumnoffset, &terminal->data->fieldid))
    {
      c = key;

      /* Instead of using the field attributes use the attributes of
       * the matching column of the field.
       */
      buf5250[subwindowcount][row - windowrowoffset][column -
						     windowcolumnoffset] =
	c | (fields5250[subwindowcount].fields[terminal->data->fieldid].
	     contents[column - windowcolumnoffset -
		      fields5250[subwindowcount].fields[terminal->data->
							fieldid].
		      column] & 0xffffff00);
      fields5250[subwindowcount].fields[terminal->data->fieldid].
	contents[column - windowcolumnoffset -
		 fields5250[subwindowcount].fields[terminal->data->fieldid].
		 column] =
	buf5250[subwindowcount][row - windowrowoffset][column -
						       windowcolumnoffset];
      if (subwindowcount == 0)
	{
	  place_character (terminal->data->win,
			   terminal->data->gcgreen,
			   terminal->data->gcwhite,
			   terminal->data->gcred,
			   terminal->data->gcblue,
			   terminal->data->gcyellow,
			   terminal->data->gcturquoise,
			   terminal->data->gcpink,
			   terminal->data->gcblack,
			   terminal->data->font_info,
			   row, column, subwindowcount,
			   buf5250[subwindowcount][row][column]);
	}
      else
	{
	  place_character (currentwin->data->win,
			   terminal->data->gcgreen,
			   terminal->data->gcwhite,
			   terminal->data->gcred,
			   terminal->data->gcblue,
			   terminal->data->gcyellow,
			   terminal->data->gcturquoise,
			   terminal->data->gcpink,
			   terminal->data->gcblack,
			   terminal->data->font_info,
			   row - windowrowoffset,
			   column - windowcolumnoffset, subwindowcount,
			   buf5250[subwindowcount][row - windowrowoffset]
			   [column - windowcolumnoffset]);
	}
    }
  return;
}
