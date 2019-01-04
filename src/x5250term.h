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



struct _Tn5250TerminalPrivate
{
  Window win;
  Window indwin;
  Window popup;
  Window *popup_menuitems;
  int popup_menuitemscount;
  Bool popped;
  Window debugpopup;
  Window debugpopup_menuitem1;
  Window debugpopup_menuitem2;
  Bool debugpopped;
  Window macropopup;
  Window *macropopup_menuitems;
  int macropopup_menuitemscount;
  Bool macropopped;
  XFontStruct *font_info;
  XFontStruct *ui_font_info;
  int fontsize;
  int font132size;
  XSizeHints *size_hints;
  unsigned int width;
  unsigned int height;		/* window size */
  GC gcgreen;
  GC gcwhite;
  GC gcred;
  GC gcblue;
  GC gcyellow;
  GC gcturquoise;
  GC gcpink;
  GC gcblack;
  GC gcui;
  GC gcmenu;
  Cursor cursor_xterm;
  Cursor cursor_watch;
  Cursor cursor_x;
  Cursor cursor_left;
  unsigned int borderwidth;	/* four pixels */
  int xconnectionfd;
  int row;
  int column;
  int fieldid;
  int currentscreencolumns;
  int keypress;
  int refreshscreen;
  int pasterow;
  int pastecolumn;
  int selectbegrow;
  int selectbegcolumn;
  int selectendrow;
  int selectendcolumn;
  char selection[3591];
  int ind_message;
  int ind_busy;
  int ind_lock;
  int ind_wait;
  int ind_insert;
  Tn5250Display *tn5250display;
  Bool alignpaste;
  Bool pastecursor;
  Bool ruler;
  Bool scrollbar;
  Bool havefocus;
  Bool showattributes;
  Bool columnseparators;
  int bellvolume;
  int quit_flag:1;
};



/* Creates the tn5250 terminal object */
Tn5250Terminal *x5250_terminal_new ();


struct _Tn5250WindowPrivate
{
  Window win;
};

struct _Tn5250MenubarPrivate
{
  Window win;
};

struct _Tn5250MenuitemPrivate
{
  Window win;
};
