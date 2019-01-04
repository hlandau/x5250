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
#include "dialog.h"
#include <stdlib.h>


#define MENUITEMS 8
#define DEBUGMENUITEMS 2
#define MAXMACRONAMELEN 256	/* Must be greater than 11 */


extern Display *display;
extern int screen_num;
extern unsigned long foreground_pixel, background_pixel, border_pixel;



/* This function creates the main menu
 */
void
createdialog (Tn5250Terminal * terminal)
{
  XSetWindowAttributes windowattribs;
  int menuitemheight;
  int dialogwidth;
  int i;

  menuitemheight =
    terminal->data->ui_font_info->max_bounds.ascent +
    terminal->data->ui_font_info->max_bounds.descent +
    (terminal->data->borderwidth * 2);

  dialogwidth = XTextWidth (terminal->data->ui_font_info,
			    "  System request key (Alt-SysRq)  ", 34);

  windowattribs.background_pixel = background_pixel;
  windowattribs.border_pixel = border_pixel;
  windowattribs.save_under = True;
  windowattribs.override_redirect = True;
  terminal->data->popup =
    XCreateWindow (display, RootWindow (display, screen_num), 0, 0,
		   dialogwidth,
		   (menuitemheight * terminal->data->popup_menuitemscount) +
		   menuitemheight, terminal->data->borderwidth,
		   CopyFromParent, CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWSaveUnder |
		   CWOverrideRedirect, &windowattribs);
  XSelectInput (display, terminal->data->popup,
		ExposureMask | ButtonPressMask);
  XDefineCursor (display, terminal->data->popup, terminal->data->cursor_left);

  terminal->data->popup_menuitems =
    malloc (sizeof (Window) * terminal->data->popup_menuitemscount);

  for (i = 0; i < terminal->data->popup_menuitemscount; i++)
    {
      terminal->data->popup_menuitems[i] =
	XCreateWindow (display, terminal->data->popup, 0,
		       menuitemheight * (i + 1),
		       dialogwidth - (terminal->data->borderwidth * 2),
		       menuitemheight, terminal->data->borderwidth,
		       CopyFromParent, CopyFromParent, CopyFromParent,
		       CWBackPixel | CWBorderPixel | CWSaveUnder,
		       &windowattribs);
      XSelectInput (display, terminal->data->popup_menuitems[i],
		    ExposureMask | ButtonPressMask);
      XMapWindow (display, terminal->data->popup_menuitems[i]);
    }

  return;
}




void
popdialog (Tn5250Terminal * terminal, XEvent * report)
{
  int x, y;

  /* Get the row and column that correspond to this button
   * press event.
   */
  x = report->xbutton.x_root;
  y = report->xbutton.y_root;

  if (terminal->data->popped == False)
    {
      XMoveWindow (display, terminal->data->popup, x, y);
      XMapRaised (display, terminal->data->popup);
      terminal->data->popped = True;
    }
  else
    {
      XUnmapWindow (display, terminal->data->popup);
      terminal->data->popped = False;
    }
  return;
}





void
drawdialog (Tn5250Terminal * terminal)
{
  char string1[50];
  int len1;
  int width1;
  int font_height;
  int dialogwidth;
  int menuitemwidth;
  int i;

  dialogwidth = XTextWidth (terminal->data->ui_font_info,
			    "  System request key (Alt-SysRq)  ", 34);
  menuitemwidth = dialogwidth - (terminal->data->borderwidth * 2);

  memset (string1, '\0', sizeof (string1));
  sprintf (string1, "%s", "Main Menu");

  /* We need length for both XTextWidth and XDrawString */
  len1 = strlen (string1);

  width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

  font_height =
    terminal->data->ui_font_info->ascent +
    terminal->data->ui_font_info->descent;

  XDrawString (display, terminal->data->popup, terminal->data->gcui,
	       (dialogwidth - width1) / 2, font_height, string1, len1);


  for (i = 0; i < terminal->data->popup_menuitemscount; i++)
    {
      memset (string1, '\0', sizeof (string1));
      switch (i)
	{
	case 0:
	  sprintf (string1, "%s", "Debug menu");
	  break;
	case 1:
	  sprintf (string1, "%s", "Macro menu");
	  break;
	case 2:
	  sprintf (string1, "%s", "Toggle ruler");
	  break;
	case 3:
	  sprintf (string1, "%s", "Help key (Ctrl-h)");
	  break;
	case 4:
	  sprintf (string1, "%s", "Reset key (Ctrl-r)");
	  break;
	case 5:
	  sprintf (string1, "%s", "System request key (Alt-SysRq)");
	  break;
	case 6:
	  sprintf (string1, "%s", "Toggle column separators");
	  break;
	case 7:
	  sprintf (string1, "%s", "Quit x5250 (Ctrl-q)");
	  break;
	}

      len1 = strlen (string1);

      width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

      XDrawString (display, terminal->data->popup_menuitems[i],
		   terminal->data->gcui, (menuitemwidth - width1) / 2,
		   font_height, string1, len1);
    }

  return;
}






/* This function creates the debug menu
 */
void
createdebugdialog (Tn5250Terminal * terminal)
{
  XSetWindowAttributes windowattribs;
  int menuitemheight;
  int dialogwidth;

  menuitemheight =
    terminal->data->ui_font_info->max_bounds.ascent +
    terminal->data->ui_font_info->max_bounds.descent +
    (terminal->data->borderwidth * 2);

  dialogwidth = XTextWidth (terminal->data->ui_font_info,
			    "  dump screen buffer  ", 22);

  windowattribs.background_pixel = background_pixel;
  windowattribs.border_pixel = border_pixel;
  windowattribs.save_under = True;
  windowattribs.override_redirect = True;
  terminal->data->debugpopup =
    XCreateWindow (display, RootWindow (display, screen_num), 0, 0,
		   dialogwidth,
		   (menuitemheight * DEBUGMENUITEMS) + menuitemheight,
		   terminal->data->borderwidth, CopyFromParent,
		   CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWSaveUnder |
		   CWOverrideRedirect, &windowattribs);
  XSelectInput (display, terminal->data->debugpopup,
		ExposureMask | ButtonPressMask);
  XDefineCursor (display, terminal->data->debugpopup,
		 terminal->data->cursor_left);

  terminal->data->debugpopup_menuitem1 =
    XCreateWindow (display, terminal->data->debugpopup, 0, menuitemheight,
		   dialogwidth - (terminal->data->borderwidth * 2),
		   menuitemheight, terminal->data->borderwidth,
		   CopyFromParent, CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWSaveUnder, &windowattribs);
  XSelectInput (display, terminal->data->debugpopup_menuitem1,
		ExposureMask | ButtonPressMask);
  XMapWindow (display, terminal->data->debugpopup_menuitem1);

  terminal->data->debugpopup_menuitem2 =
    XCreateWindow (display, terminal->data->debugpopup, 0, menuitemheight * 2,
		   dialogwidth - (terminal->data->borderwidth * 2),
		   menuitemheight, terminal->data->borderwidth,
		   CopyFromParent, CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWSaveUnder, &windowattribs);
  XSelectInput (display, terminal->data->debugpopup_menuitem2,
		ExposureMask | ButtonPressMask);
  XMapWindow (display, terminal->data->debugpopup_menuitem2);
  return;
}




void
popdebugdialog (Tn5250Terminal * terminal, XEvent * report)
{
  int x, y;

  /* Get the row and column that correspond to this button
   * press event.
   */
  x = report->xbutton.x_root;
  y = report->xbutton.y_root;

  if (terminal->data->debugpopped == False)
    {
      XMoveWindow (display, terminal->data->debugpopup, x, y);
      XMapRaised (display, terminal->data->debugpopup);
      terminal->data->debugpopped = True;
    }
  else
    {
      XUnmapWindow (display, terminal->data->debugpopup);
      terminal->data->debugpopped = False;
    }
  return;
}





void
drawdebugdialog (Tn5250Terminal * terminal)
{
  char string1[25];
  int len1;
  int width1;
  int font_height;
  int dialogwidth;
  int menuitemwidth;

  dialogwidth = XTextWidth (terminal->data->ui_font_info,
			    "  Dump screen buffer  ", 22);
  menuitemwidth = dialogwidth - (terminal->data->borderwidth * 2);

  memset (string1, '\0', sizeof (string1));
  sprintf (string1, "%s", "Debug Menu");

  /* We need length for both XTextWidth and XDrawString */
  len1 = strlen (string1);

  width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

  font_height =
    terminal->data->ui_font_info->ascent +
    terminal->data->ui_font_info->descent;

  XDrawString (display, terminal->data->debugpopup, terminal->data->gcui,
	       (dialogwidth - width1) / 2, font_height, string1, len1);


  memset (string1, '\0', sizeof (string1));
  sprintf (string1, "%s", "Toggle attributes");

  len1 = strlen (string1);

  width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

  XDrawString (display, terminal->data->debugpopup_menuitem1,
	       terminal->data->gcui, (menuitemwidth - width1) / 2,
	       font_height, string1, len1);


  memset (string1, '\0', sizeof (string1));
  sprintf (string1, "%s", "Dump screen buffer");

  len1 = strlen (string1);

  width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

  XDrawString (display, terminal->data->debugpopup_menuitem2,
	       terminal->data->gcui, (menuitemwidth - width1) / 2,
	       font_height, string1, len1);
  return;
}



/* This function creates the macro menu
 */
void
createmacrodialog (Tn5250Terminal * terminal, Tn5250Macro * macro)
{
#ifdef MACRO_SUPPORT
  XSetWindowAttributes windowattribs;
  int menuitemheight;
  int dialogwidth;
  int i;
  int totalmacros = 0;
  int longnamelen = 0;
  int longnameidx = -1;

  for (i = 0; i < 24; i++)
    {
      if (macro->data[i].BuffM != NULL)
	{
	  totalmacros++;
	}
      if ((macro->data[i].name != NULL)
	  && (strlen (macro->data[i].name) > longnamelen))
	{
	  longnamelen = strlen (macro->data[i].name);
	  longnameidx = i;
	}
    }

  if (totalmacros == 0)
    {
      return;
    }

  terminal->data->macropopup_menuitems =
    malloc (sizeof (Window) * totalmacros);
  terminal->data->macropopup_menuitemscount = totalmacros;

  menuitemheight =
    terminal->data->ui_font_info->max_bounds.ascent +
    terminal->data->ui_font_info->max_bounds.descent +
    (terminal->data->borderwidth * 2);

  if (longnameidx > -1)
    {
      unsigned char namestring[MAXMACRONAMELEN] = { '\0' };

      if (strlen (macro->data[i].name) > MAXMACRONAMELEN - 5)
	{
	  unsigned char tmpstring[MAXMACRONAMELEN] = { '\0' };

	  strncpy (tmpstring, macro->data[longnameidx].name,
		   MAXMACRONAMELEN - 8);
	  sprintf (namestring, "  %s...  ", tmpstring);
	}
      else
	{
	  sprintf (namestring, "  %s  ", macro->data[longnameidx].name);
	}

      dialogwidth = XTextWidth (terminal->data->ui_font_info,
				namestring, strlen (namestring));
    }
  else
    {
      dialogwidth = XTextWidth (terminal->data->ui_font_info,
				"  Run macro WW  ", 16);
    }

  windowattribs.background_pixel = background_pixel;
  windowattribs.border_pixel = border_pixel;
  windowattribs.save_under = True;
  windowattribs.override_redirect = True;
  terminal->data->macropopup =
    XCreateWindow (display, RootWindow (display, screen_num), 0, 0,
		   dialogwidth,
		   (menuitemheight * totalmacros) + menuitemheight,
		   terminal->data->borderwidth, CopyFromParent,
		   CopyFromParent, CopyFromParent,
		   CWBackPixel | CWBorderPixel | CWSaveUnder |
		   CWOverrideRedirect, &windowattribs);
  XSelectInput (display, terminal->data->macropopup,
		ExposureMask | ButtonPressMask);
  XDefineCursor (display, terminal->data->macropopup,
		 terminal->data->cursor_left);

  for (i = 0; i < totalmacros; i++)
    {
      terminal->data->macropopup_menuitems[i] =
	XCreateWindow (display, terminal->data->macropopup, 0,
		       menuitemheight * (i + 1),
		       dialogwidth - (terminal->data->borderwidth * 2),
		       menuitemheight, terminal->data->borderwidth,
		       CopyFromParent, CopyFromParent, CopyFromParent,
		       CWBackPixel | CWBorderPixel | CWSaveUnder,
		       &windowattribs);
      XSelectInput (display, terminal->data->macropopup_menuitems[i],
		    ExposureMask | ButtonPressMask);
      XMapWindow (display, terminal->data->macropopup_menuitems[i]);
    }
#endif
  return;
}




void
popmacrodialog (Tn5250Terminal * terminal, XEvent * report)
{
  int x, y;

  /* Get the row and column that correspond to this button
   * press event.
   */
  x = report->xbutton.x_root;
  y = report->xbutton.y_root;

  if (terminal->data->macropopped == False)
    {
      XMoveWindow (display, terminal->data->macropopup, x, y);
      XMapRaised (display, terminal->data->macropopup);
      terminal->data->macropopped = True;
    }
  else
    {
      XUnmapWindow (display, terminal->data->macropopup);
      terminal->data->macropopped = False;
    }
  return;
}



void
drawmacrodialog (Tn5250Terminal * terminal, Tn5250Macro * macro)
{
#ifdef MACRO_SUPPORT
  char string1[25];
  int len1;
  int width1;
  int font_height;
  int dialogwidth;
  int menuitemwidth;
  int longnamelen = 0;
  int longnameidx = -1;
  int i;
  unsigned char macronames[24][MAXMACRONAMELEN];

  if (terminal->data->macropopup_menuitemscount == 0)
    {
      return;
    }

  int curmacro = 0;

  for (i = 0; i < 24; i++)
    {
      if (macro->data[i].name != NULL)
	{
	  if (strlen (macro->data[i].name) > MAXMACRONAMELEN - 5)
	    {
	      unsigned char tmpstring[MAXMACRONAMELEN] = { '\0' };

	      strncpy (tmpstring, macro->data[i].name, MAXMACRONAMELEN - 8);
	      sprintf (macronames[curmacro], "  %s...  ", tmpstring);
	    }
	  else
	    {
	      sprintf (macronames[curmacro], "  %s  ", macro->data[i].name);
	    }

	  if (strlen (macronames[curmacro]) > longnamelen)
	    {
	      longnamelen = strlen (macronames[curmacro]);
	      longnameidx = curmacro;
	    }

	  curmacro++;

	  if (curmacro >= terminal->data->macropopup_menuitemscount)
	    {
	      break;
	    }
	}
      else if (macro->data[i].BuffM != NULL)
	{
	  sprintf (macronames[curmacro], "  Run macro %d  ", i + 1);
	  curmacro++;
	}
    }

  if (longnameidx > -1)
    {
      char *namestring =
	(char *) calloc (strlen (macronames[longnameidx]) + 1,
			 sizeof (char));
      sprintf (namestring, "%s", macronames[longnameidx]);
      dialogwidth = XTextWidth (terminal->data->ui_font_info,
				namestring, strlen (namestring));
      free (namestring);
    }
  else
    {
      dialogwidth = XTextWidth (terminal->data->ui_font_info,
				"  Run macro WW  ", 16);
    }

  menuitemwidth = dialogwidth - (terminal->data->borderwidth * 2);

  memset (string1, '\0', sizeof (string1));
  sprintf (string1, "%s", "Macro Menu");

  /* We need length for both XTextWidth and XDrawString */
  len1 = strlen (string1);

  width1 = XTextWidth (terminal->data->ui_font_info, string1, len1);

  font_height =
    terminal->data->ui_font_info->ascent +
    terminal->data->ui_font_info->descent;

  XDrawString (display, terminal->data->macropopup, terminal->data->gcui,
	       (dialogwidth - width1) / 2, font_height, string1, len1);


  for (i = 0; i < terminal->data->macropopup_menuitemscount; i++)
    {
      len1 = strlen (macronames[i]);

      width1 = XTextWidth (terminal->data->ui_font_info, macronames[i], len1);

      XDrawString (display, terminal->data->macropopup_menuitems[i],
		   terminal->data->gcui, (menuitemwidth - width1) / 2,
		   font_height, macronames[i], len1);
    }

#endif
  return;
}
