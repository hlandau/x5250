/*
 * Copyright 2003 James Rich
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#ifdef SUN
#include <X11/Sunkeysym.h>
#endif
#include <X11/Xresource.h>

#define _TN5250_TERMINAL_PRIVATE_DEFINED
#define _TN5250_WINDOW_PRIVATE_DEFINED
#define _TN5250_MENU_PRIVATE_DEFINED
#include <tn5250.h>


#define BITMAPDEPTH 1


/* Color definitions. */
#define A_X5250_CONTENTMASK	0x000000ff
#define A_X5250_GREEN		0x00000000
#define A_X5250_WHITE		0x00000100
#define A_X5250_RED		0x00000200
#define A_X5250_TURQUOISE	0x00000300
#define A_X5250_YELLOW		0x00000400
#define A_X5250_PINK		0x00000500
#define A_X5250_BLUE		0x00000600
#define A_X5250_BLACK		0x00000700
#define A_X5250_COLORMASK      	0x00000f00
#define A_X5250_NORMAL		0x00000000
#define A_X5250_COLUMNSEP	0x00001000
#define A_X5250_COLUMNSEPMASK  	0x0000f000
#define A_X5250_BOLD		0x00010000
#define A_X5250_BOLDMASK       	0x000f0000
#define A_X5250_REVERSE		0x00100000
#define A_X5250_REVERSEMASK    	0x00f00000
#define A_X5250_FONTMASK  	0x00ff0000
#define A_X5250_UNDERLINE	0x01000000
#define A_X5250_UNDERLINEMASK  	0x0f000000
#define A_X5250_NONDISPLAY	0x10000000
#define A_X5250_NONDISPLAYMASK 	0xf0000000
#define A_X5250_ATTRIBUTEMASK  	0xfffff000


/* Amount of space (in pixels) to place between rows on the screen */
#define ROWSPACING 4

/* Amount of space (in pixels) to place below the font baseline and
 * the field underline
 */
#define BASELINESPACE 3

/* Amount of space (in pixels) to place around the edges of the screen */
#define WINDOWMARGIN 5


/* values for window_size in main, is window big enough to be useful? */
#define SMALL 1
#define OK 0


/* This struct contains all the elements necessary to define a 5250
 * field.  The contents member uses the same masks as buf5250 to
 * define color and font attributes.
 */
typedef struct _field5250
{
  unsigned int fieldid;		/* Numeric ID of this field */
  short inputcapable;		/* Boolean input capable */
  short continuous;		/* Boolean is a continuous field */
  short wordwrap;		/* Boolean is a word wrap field */
  unsigned int attributes;	/* Field attributes (same as the
				 * attributes of buf5250).  These are
				 * for the whole field, but may be
				 * overridden by the contents member */
  unsigned int row;		/* Row field starts on */
  unsigned int column;		/* Column field starts on */
  unsigned int length;		/* length (in characters) of field */
  unsigned int nextfieldprogressionid;	/* Field ID of next field cursor
					 * should move to */
  unsigned int contents[3564];	/* Contents of field (same as the
				 * contents of buf5250 for the
				 * same row and column) */
} field5250;


/* This struct contains all the fields that appear (either on top or
 * obscured by windows) on a screen.
 *
 * According to the docs the maximum number of input fields that can
 * exist on a screen is 256.
 */
typedef struct _screenfields
{
  unsigned int totalfields;
  field5250 fields[256];
} screenfields;


/* This struct defines the key map.
 */
struct _keymap;

struct _keymap
{
  struct _keymap *next;
  struct _keymap *prev;
  KeySym keysym;
  unsigned int state;
  int tn5250key;
};

typedef struct _keymap keymap;



/* allocate some colors. */
int alloccolors ();

/* This draws just a single character in the buf5250 buffer on the screen
 * (i.e. the one at location row and column).  This is called after each
 * keypress event.
 */
void place_character (Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
		      GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
		      XFontStruct * font_info, int row, int column,
		      int bufferid, unsigned int character);

/* This draws the entire contents of buf5250 onto the screen.  This
 * is called when we receive expose events.
 */
void place_text (Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
		 GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
		 XFontStruct * font_info, unsigned int win_width,
		 unsigned int win_height, int bufferid);

/* This draws all the graphical elements of the screen.  This mostly
 * consists of the underlines that indicate 5250 fields.  This is called
 * after receiving an expose event.
 */
void place_graphics (Tn5250Terminal * terminal, Window win, int bufferid);

/* Draw a caret (cursor) */
void draw_caret (Window win, GC gc, XFontStruct * font_info, int row,
		 int column, Bool havefocus, Bool ruler, int bufferid);

/* Erase the caret */
void erase_caret (Tn5250Terminal * terminal,
		  Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
		  GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
		  XFontStruct * font_info,  int row, int column, Bool ruler,
		  int bufferid);

/* This handles keypress events */
int keyhandler (int *fieldcount, int *row, int *column, XEvent * report,
		KeySym * keysym, XComposeStatus * compose,
		unsigned char * buffer,	int windowrowoffset,
		int windowcolumnoffset);

/* This handles button press events */
int buttonpresshandler (Tn5250Terminal * terminal, XEvent * report);

/* This handles button release events */
int buttonreleasehandler (Tn5250Terminal * terminal, XEvent * report);

/* This handles button motion events */
void buttonmotionhandler (Tn5250Terminal * terminal, XEvent * report);

/* This loads the key map */
void load_keymap ();

/* This clears a screen before it is drawn to */
void destroy_screen (int windowid);

/* This is just a little something to let us know if the window is too
 * small to hold everything we want to draw.
 */
void TooSmall (Window win, GC gc, XFontStruct * font_info);

/* This creates a graphics context with which to draw.  The instance
 * argument is used to differentiate between calls.  Different instances
 * create different graphics contexts with different colors.
 */
void get_GC (Window win, GC * gc, XFontStruct * font_info, int instance);

/* Loads a (at this point hard-coded) font.  If the bold argument is zero
 * then load a normal font.  Otherwise load a bold font.
 */
void load_font (XFontStruct ** font_info, char *font, int pointsize);

/* Gets the text to paste from the X server */
void paste_text (Window win, Atom property, Tn5250Terminal * terminal);

/* Handles selection request events to send to clients */
void selectionhandler (XEvent * report, Tn5250Terminal * terminal);

/* Tells if the given cursor position is in a field.  The field ID is
 * returned in the fieldid parameter.
 */
int is_in_field (int row, int column, int *fieldid);

/* Draws the indicator window with the system indicator text or icons.
 */
void draw_indicator_win (Tn5250Terminal * terminal);

/* This draws any 5250 scrollbars */
void draw_5250_scrollbar (Tn5250Terminal * terminal);

/* This draws any 5250 menus */
void draw_5250_menubar (Tn5250Terminal * terminal);
