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
#include "debug.h"
#include <stdlib.h>



extern Display *display;
extern unsigned int ***buf5250;
extern int subwindowcount;
extern int screencolumns;
extern int screenrows;

extern unsigned long foreground_pixel, background_pixel, border_pixel;
extern unsigned long red_pixel, green_pixel, blue_pixel, black_pixel,
  white_pixel, yellow_pixel, turquoise_pixel, pink_pixel, cursor_pixel;



/* This function handles button press events
 */
int
buttonpresshandler (Tn5250Terminal * terminal, XEvent * report)
{
  Tn5250Window *windowlist, *subwindow;
  unsigned int button;
  Atom data_prop;
  int fontwidth, fontheight;
  int i;

  button = report->xbutton.button;
  switch (button)
    {
    case 1:

      /* clicked on a macro menu */
      for (i = 0; i < terminal->data->macropopup_menuitemscount; i++)
	{
	  if (report->xexpose.window == terminal->data->macropopup_menuitems[i])
	    {
	      popmacrodialog (terminal, report);
	      popdialog (terminal, report);
	      break;
	    }
	}

      /* clicked on debug menu */
      if (report->xexpose.window == terminal->data->popup_menuitems[0])
	{
	  popdebugdialog (terminal, report);
	  break;
	}

      /* clicked on macro menu */
      if (report->xexpose.window == terminal->data->popup_menuitems[1])
	{
	  popmacrodialog (terminal, report);
	  break;
	}

      /* clicked on ruler toggle */
      else if (report->xexpose.window == terminal->data->popup_menuitems[2])
	{
	  popdialog (terminal, report);
	  if (terminal->data->ruler == False)
	    {
	      terminal->data->ruler = True;
	    }
	  else
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
			       terminal->data->gcwhite, terminal->data->gcred,
			       terminal->data->gcblue,
			       terminal->data->gcyellow,
			       terminal->data->gcturquoise,
			       terminal->data->gcpink,
			       terminal->data->gcblack,
			       terminal->data->font_info, terminal->data->row,
			       terminal->data->column, terminal->data->ruler,
			       subwindowcount);
		}
	      terminal->data->ruler = False;
	    }
	}

      /* clicked on help */
      else if (report->xexpose.window == terminal->data->popup_menuitems[3])
	{
	  popdialog (terminal, report);
	  terminal->data->keypress = K_HELP;
	  terminal->data->refreshscreen = 1;
	}

      /* clicked on reset */
      else if (report->xexpose.window == terminal->data->popup_menuitems[4])
	{
	  popdialog (terminal, report);
	  terminal->data->keypress = K_RESET;
	  terminal->data->refreshscreen = 1;
	}

      /* clicked on system request */
      else if (report->xexpose.window == terminal->data->popup_menuitems[5])
	{
	  popdialog (terminal, report);
	  terminal->data->keypress = K_SYSREQ;
	  terminal->data->refreshscreen = 1;
	}

      /* clicked oncolumn seps toggle */
      else if (report->xexpose.window == terminal->data->popup_menuitems[6])
	{
	  popdialog (terminal, report);
	  if (terminal->data->columnseparators == False)
	    {
	      terminal->data->columnseparators = True;
	    }
	  else
	    {
	      terminal->data->columnseparators = False;
	    }
	  terminal->data->refreshscreen = 1;
	}

      /* clicked on quit */
      else if (report->xexpose.window == terminal->data->popup_menuitems[7])
	{
	  return (0);
	}

      /* clicked on attribs toggle */
      else if (report->xexpose.window == terminal->data->debugpopup_menuitem1)
	{
	  popdebugdialog (terminal, report);
	  popdialog (terminal, report);
	  if (terminal->data->showattributes == False)
	    {
	      terminal->data->showattributes = True;
	    }
	  else
	    {
	      terminal->data->showattributes = False;
	    }
	  terminal->data->refreshscreen = 1;
	}

      /* clicked on screen dump */
      else if (report->xexpose.window == terminal->data->debugpopup_menuitem2)
	{
	  popdebugdialog (terminal, report);
	  popdialog (terminal, report);
	  dump_buffer (subwindowcount);
	  terminal->data->refreshscreen = 1;
	}

      else if (report->xexpose.window == terminal->data->popup)
	{
	  break;
	}
      else if (report->xexpose.window == terminal->data->debugpopup)
	{
	  break;
	}
      else if (report->xexpose.window == terminal->data->macropopup)
	{
	  break;
	}
      else if (report->xexpose.window == terminal->data->indwin)
	{
	  break;
	}
      else
	{
	  /* If the user clicked on an attribute pop up a windows that
	   * describes the current attributes.
	   */
	  if ((terminal->data->showattributes == True) &&
	      (buf5250[subwindowcount][terminal->data->selectbegrow]
	       [terminal->data->selectbegcolumn] ==
	       ('@' | A_X5250_PINK | A_X5250_NORMAL)))
	    {
	      break;
	    }
	  /* Redraw the screen to get rid of any previous highlighting */
	  fontwidth = terminal->data->font_info->max_bounds.width;
	  fontheight =
	    terminal->data->font_info->max_bounds.ascent +
	    terminal->data->font_info->max_bounds.descent;
	  XClearArea (display, terminal->data->win,
		      (fontwidth * terminal->data->selectbegcolumn) + 1,
		      ((fontheight +
			ROWSPACING) * terminal->data->selectbegrow) +
		      BASELINESPACE + 3,
		      (fontwidth * terminal->data->selectendcolumn) + 1,
		      ((fontheight +
			ROWSPACING) * terminal->data->selectendrow) +
		      BASELINESPACE + 3, False);
	  place_text (terminal->data->win, terminal->data->gcgreen,
		      terminal->data->gcwhite, terminal->data->gcred,
		      terminal->data->gcblue, terminal->data->gcyellow,
		      terminal->data->gcturquoise, terminal->data->gcpink,
		      terminal->data->gcblack, terminal->data->font_info,
		      terminal->data->width, terminal->data->height, 0);
	  place_graphics (terminal, terminal->data->win, 0);

	  if (subwindowcount > 0)
	    {
	      windowlist =
		terminal->data->tn5250display->display_buffers->window_list;

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
		  place_graphics (terminal, subwindow->data->win, i);
		}
	    }

	  if (subwindowcount == 0)
	    {
	      draw_caret (terminal->data->win,
			  terminal->data->gcyellow,
			  terminal->data->font_info,
			  terminal->data->row, terminal->data->column, True,
			  terminal->data->ruler, subwindowcount);
	    }
	  else
	    {
	      windowlist =
		terminal->data->tn5250display->display_buffers->window_list;
	      subwindow =
		tn5250_window_list_find_by_id (windowlist,
					       subwindowcount - 1);
	      draw_caret (subwindow->data->win, terminal->data->gcyellow,
			  terminal->data->font_info, terminal->data->row,
			  terminal->data->column, True, terminal->data->ruler,
			  subwindowcount);
	    }

	  /* This is the start of a selection */

	  terminal->data->selectbegcolumn =
	    (report->xbutton.x -
	     1) / terminal->data->font_info->max_bounds.width;
	  terminal->data->selectbegrow =
	    (report->xbutton.y /
	     (terminal->data->font_info->max_bounds.ascent +
	      terminal->data->font_info->max_bounds.descent + ROWSPACING));

	  XSetForeground (display, terminal->data->gcgreen, background_pixel);
	  XSetBackground (display, terminal->data->gcgreen, green_pixel);
	  XSetForeground (display, terminal->data->gcwhite, background_pixel);
	  XSetBackground (display, terminal->data->gcwhite, white_pixel);
	  XSetForeground (display, terminal->data->gcred, background_pixel);
	  XSetBackground (display, terminal->data->gcred, red_pixel);
	  XSetForeground (display, terminal->data->gcblue, background_pixel);
	  XSetBackground (display, terminal->data->gcblue, blue_pixel);
	  XSetForeground (display, terminal->data->gcyellow,
			  background_pixel);
	  XSetBackground (display, terminal->data->gcyellow, yellow_pixel);
	  XSetForeground (display, terminal->data->gcturquoise,
			  background_pixel);
	  XSetBackground (display, terminal->data->gcturquoise,
			  turquoise_pixel);
	  XSetForeground (display, terminal->data->gcpink, background_pixel);
	  XSetBackground (display, terminal->data->gcpink, pink_pixel);
	  XSetForeground (display, terminal->data->gcblack, background_pixel);
	  XSetBackground (display, terminal->data->gcblack, black_pixel);
	}
      break;

    case 2:
      for (i = 0; i < terminal->data->popup_menuitemscount; i++)
	{
	  if (report->xexpose.window == terminal->data->popup_menuitems[i])
	    {
	      break;
	    }
	}
      if ((report->xexpose.window == terminal->data->popup)
	  || (report->xexpose.window == terminal->data->indwin))
	{
	  break;
	}
      else
	{
	  if ((report->xkey.state & ShiftMask) != 0)
	    {
	      terminal->data->alignpaste = True;
	    }
	  else
	    {
	      terminal->data->alignpaste = False;
	    }
	  /* Get the row and column that correspond to this button
	   * press event.
	   */
	  if (terminal->data->pastecursor == True)
	    {
	      terminal->data->pastecolumn = terminal->data->column;
	      terminal->data->pasterow = terminal->data->row;
	    }
	  else
	    {
	      terminal->data->pastecolumn =
		(report->xbutton.x -
		 1) / terminal->data->font_info->max_bounds.width;
	      terminal->data->pasterow =
		(report->xbutton.y /
		 (terminal->data->font_info->max_bounds.ascent +
		  terminal->data->font_info->max_bounds.descent +
		  ROWSPACING));
	    }

	  /* If the current cursor location is not in a field, get outta here
	   * (this needs to be changed to indicate input prohibited)
	   */
	  if (!
	      (is_in_field
	       (terminal->data->pasterow, terminal->data->pastecolumn,
		&terminal->data->fieldid)))
	    {
	      return (-1);
	    }

#ifdef DEBUG
	  fprintf (stderr, "paste column: %d, row: %d, fieldid: %d\n",
		   terminal->data->pastecolumn, terminal->data->pasterow,
		   terminal->data->fieldid);
#endif

	  tn5250_dbuffer_cursor_set (terminal->data->tn5250display->
				     display_buffers,
				     terminal->data->pasterow,
				     terminal->data->pastecolumn);

	  /* Get the selected text */
	  data_prop = XInternAtom (display, "PASTED_TEXT", False);
	  if (subwindowcount == 0)
	    {
	      XConvertSelection (display, XA_PRIMARY, XA_STRING, data_prop,
				 terminal->data->win, report->xbutton.time);
	    }
	  else
	    {
	      windowlist =
		terminal->data->tn5250display->display_buffers->window_list;
	      subwindow =
		tn5250_window_list_find_by_id (windowlist,
					       subwindowcount - 1);
	      XConvertSelection (display, XA_PRIMARY, XA_STRING, data_prop,
				 subwindow->data->win, report->xbutton.time);
	    }
	}
      break;

    case 3:
      if (terminal->data->debugpopped == True)
	{
	  popdebugdialog (terminal, report);
	}
      if (terminal->data->macropopped == True)
	{
	  popmacrodialog (terminal, report);
	}
      popdialog (terminal, report);
      break;

    default:
      break;
    }
  return (button);
}




/* This function handles button motion events
 */
void
buttonmotionhandler (Tn5250Terminal * terminal, XEvent * report)
{
  Tn5250Window *windowlist, *subwindow;
  unsigned int character;
  static int maxcolumn = 0;
  static int maxrow = 0;
  int i, j;

  terminal->data->selectendcolumn =
    (report->xmotion.x - 1) / terminal->data->font_info->max_bounds.width;
  terminal->data->selectendrow =
    (report->xmotion.y /
     (terminal->data->font_info->max_bounds.ascent +
      terminal->data->font_info->max_bounds.descent + ROWSPACING));

  if ((terminal->data->selectendcolumn <= terminal->data->selectbegcolumn) ||
      (terminal->data->selectendrow < terminal->data->selectbegrow))
    {
      return;
    }

  if (terminal->data->selectendrow > tn5250_terminal_height (terminal))
    {
      terminal->data->selectendrow = tn5250_terminal_height (terminal);
    }

  if (terminal->data->selectendcolumn > tn5250_terminal_width (terminal))
    {
      terminal->data->selectendcolumn = tn5250_terminal_width (terminal);
    }


  /* This does the highlighting (the graphics context has already been changed
   * by the ButtonPress event).
   */
  for (i = terminal->data->selectbegrow;
       (i <= terminal->data->selectendrow)
       && (i < tn5250_terminal_height (terminal)); i++)
    {
      for (j = terminal->data->selectbegcolumn;
	   j < terminal->data->selectendcolumn; j++)
	{
	  if (buf5250[subwindowcount][i][j] == 0)
	    {
	      character = ' ' | A_X5250_NORMAL;
	    }
	  else
	    {
	      if ((buf5250[subwindowcount][i][j] & A_X5250_NONDISPLAYMASK) !=
		  0)
		{
		  character = ' ' | A_X5250_NORMAL;
		}
	      else
		{
		  character = buf5250[subwindowcount][i][j];
		}
	    }
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
			       i, j, subwindowcount, character);
	    }
	  else
	    {
	      windowlist =
		terminal->data->tn5250display->display_buffers->window_list;
	      subwindow =
		tn5250_window_list_find_by_id (windowlist,
					       subwindowcount - 1);
	      place_character (subwindow->data->win, terminal->data->gcgreen,
			       terminal->data->gcwhite, terminal->data->gcred,
			       terminal->data->gcblue,
			       terminal->data->gcyellow,
			       terminal->data->gcturquoise,
			       terminal->data->gcpink,
			       terminal->data->gcblack,
			       terminal->data->font_info, i, j,
			       subwindowcount, character);
	    }
	}
    }


  /* This unhighlights text if the user shrinks the size of the selection area.
   * We have to set the graphics context manually here.
   *
   * First, unhighlight any rows that may have been shortened from the selection.
   */
  if ((terminal->data->selectendrow < maxrow)
      || (terminal->data->selectendcolumn < maxcolumn))
    {
      XSetBackground (display, terminal->data->gcgreen, background_pixel);
      XSetForeground (display, terminal->data->gcgreen, green_pixel);
      XSetBackground (display, terminal->data->gcwhite, background_pixel);
      XSetForeground (display, terminal->data->gcwhite, white_pixel);
      XSetBackground (display, terminal->data->gcred, background_pixel);
      XSetForeground (display, terminal->data->gcred, red_pixel);
      XSetBackground (display, terminal->data->gcblue, background_pixel);
      XSetForeground (display, terminal->data->gcblue, blue_pixel);
      XSetBackground (display, terminal->data->gcyellow, background_pixel);
      XSetForeground (display, terminal->data->gcyellow, yellow_pixel);
      XSetBackground (display, terminal->data->gcturquoise, background_pixel);
      XSetForeground (display, terminal->data->gcturquoise, turquoise_pixel);
      XSetBackground (display, terminal->data->gcpink, background_pixel);
      XSetForeground (display, terminal->data->gcpink, pink_pixel);
      XSetBackground (display, terminal->data->gcblack, background_pixel);
      XSetForeground (display, terminal->data->gcblack, black_pixel);
    }

  if (terminal->data->selectendrow < maxrow)
    {
      for (i = terminal->data->selectendrow;
	   (i <= maxrow) && (i < tn5250_terminal_height (terminal)); i++)
	{
	  for (j = terminal->data->selectbegcolumn; j < maxcolumn; j++)
	    {
	      if (buf5250[subwindowcount][i][j] == 0)
		{
		  character = ' ' | A_X5250_NORMAL;
		}
	      else
		{
		  if ((buf5250[subwindowcount][i][j] & A_X5250_NONDISPLAYMASK)
		      != 0)
		    {
		      character = ' ' | A_X5250_NORMAL;
		    }
		  else
		    {
		      character = buf5250[subwindowcount][i][j];
		    }
		}
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
				   i, j, subwindowcount, character);
		}
	      else
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;
		  subwindow =
		    tn5250_window_list_find_by_id (windowlist,
						   subwindowcount - 1);
		  place_character (subwindow->data->win,
				   terminal->data->gcgreen,
				   terminal->data->gcwhite,
				   terminal->data->gcred,
				   terminal->data->gcblue,
				   terminal->data->gcyellow,
				   terminal->data->gcturquoise,
				   terminal->data->gcpink,
				   terminal->data->gcblack,
				   terminal->data->font_info, i, j,
				   subwindowcount, character);
		}
	    }
	}
    }

  /* Then remove any columns that have been removed from the selection. */
  if (terminal->data->selectendcolumn <= maxcolumn)
    {
      for (i = terminal->data->selectbegrow;
	   (i <= terminal->data->selectendrow)
	   && (i < tn5250_terminal_height (terminal)); i++)
	{
	  for (j = terminal->data->selectendcolumn; j < maxcolumn; j++)
	    {
	      if (buf5250[subwindowcount][i][j] == 0)
		{
		  character = ' ' | A_X5250_NORMAL;
		}
	      else
		{
		  if ((buf5250[subwindowcount][i][j] & A_X5250_NONDISPLAYMASK)
		      != 0)
		    {
		      character = ' ' | A_X5250_NORMAL;
		    }
		  else
		    {
		      character = buf5250[subwindowcount][i][j];
		    }
		}
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
				   i, j, subwindowcount, character);
		}
	      else
		{
		  windowlist =
		    terminal->data->tn5250display->display_buffers->
		    window_list;
		  subwindow =
		    tn5250_window_list_find_by_id (windowlist,
						   subwindowcount - 1);
		  place_character (subwindow->data->win,
				   terminal->data->gcgreen,
				   terminal->data->gcwhite,
				   terminal->data->gcred,
				   terminal->data->gcblue,
				   terminal->data->gcyellow,
				   terminal->data->gcturquoise,
				   terminal->data->gcpink,
				   terminal->data->gcblack,
				   terminal->data->font_info, i, j,
				   subwindowcount, character);
		}
	    }
	}
    }

  /* And finally put the graphics context back to highlight mode. */
  if ((terminal->data->selectendrow < maxrow)
      || (terminal->data->selectendcolumn < maxcolumn))
    {
      XSetForeground (display, terminal->data->gcgreen, background_pixel);
      XSetBackground (display, terminal->data->gcgreen, green_pixel);
      XSetForeground (display, terminal->data->gcwhite, background_pixel);
      XSetBackground (display, terminal->data->gcwhite, white_pixel);
      XSetForeground (display, terminal->data->gcred, background_pixel);
      XSetBackground (display, terminal->data->gcred, red_pixel);
      XSetForeground (display, terminal->data->gcblue, background_pixel);
      XSetBackground (display, terminal->data->gcblue, blue_pixel);
      XSetForeground (display, terminal->data->gcyellow, background_pixel);
      XSetBackground (display, terminal->data->gcyellow, yellow_pixel);
      XSetForeground (display, terminal->data->gcturquoise, background_pixel);
      XSetBackground (display, terminal->data->gcturquoise, turquoise_pixel);
      XSetForeground (display, terminal->data->gcpink, background_pixel);
      XSetBackground (display, terminal->data->gcpink, pink_pixel);
      XSetForeground (display, terminal->data->gcblack, background_pixel);
      XSetBackground (display, terminal->data->gcblack, black_pixel);
    }
  maxrow = terminal->data->selectendrow;
  maxcolumn = terminal->data->selectendcolumn;
  return;
}




/* This function handles button release events
 */
int
buttonreleasehandler (Tn5250Terminal * terminal, XEvent * report)
{
  Tn5250Window *windowlist, *subwindow;
  unsigned int button;
  int i, j, k;

  button = report->xbutton.button;

  if (button != 1)
    {
      return (button);
    }

  for (i = 0; i < terminal->data->popup_menuitemscount; i++)
    {
      if (report->xexpose.window == terminal->data->popup_menuitems[i])
	{
	  return (button);
	}
    }
  if ((report->xexpose.window == terminal->data->popup)
      || (report->xexpose.window == terminal->data->indwin))
    {
      return (button);
    }

  terminal->data->selectendcolumn =
    (report->xbutton.x - 1) / terminal->data->font_info->max_bounds.width;
  terminal->data->selectendrow =
    (report->xbutton.y /
     (terminal->data->font_info->max_bounds.ascent +
      terminal->data->font_info->max_bounds.descent + ROWSPACING));

  XSetBackground (display, terminal->data->gcgreen, background_pixel);
  XSetForeground (display, terminal->data->gcgreen, green_pixel);
  XSetBackground (display, terminal->data->gcwhite, background_pixel);
  XSetForeground (display, terminal->data->gcwhite, white_pixel);
  XSetBackground (display, terminal->data->gcred, background_pixel);
  XSetForeground (display, terminal->data->gcred, red_pixel);
  XSetBackground (display, terminal->data->gcblue, background_pixel);
  XSetForeground (display, terminal->data->gcblue, blue_pixel);
  XSetBackground (display, terminal->data->gcyellow, background_pixel);
  XSetForeground (display, terminal->data->gcyellow, yellow_pixel);
  XSetBackground (display, terminal->data->gcturquoise, background_pixel);
  XSetForeground (display, terminal->data->gcturquoise, turquoise_pixel);
  XSetBackground (display, terminal->data->gcpink, background_pixel);
  XSetForeground (display, terminal->data->gcpink, pink_pixel);
  XSetBackground (display, terminal->data->gcblack, background_pixel);
  XSetForeground (display, terminal->data->gcblack, black_pixel);

  memset (terminal->data->selection, '\0',
	  sizeof (terminal->data->selection));

  if ((terminal->data->selectendcolumn <= terminal->data->selectbegcolumn) ||
      (terminal->data->selectendrow < terminal->data->selectbegrow))
    {
      return (button);
    }

  k = 0;
  for (i = terminal->data->selectbegrow;
       (i <= terminal->data->selectendrow) && (i < 27); i++)
    {
      /* Append a newline to the selection unless this is the last iteration. */
      if (j == terminal->data->selectendcolumn)
	{
	  terminal->data->selection[k] = '\n';
	  k++;
	}
      for (j = terminal->data->selectbegcolumn;
	   j < terminal->data->selectendcolumn; j++)
	{
	  if ((buf5250[subwindowcount][i][j] & A_X5250_CONTENTMASK) == 0)
	    {
	      terminal->data->selection[k] = ' ';
	    }
	  else
	    {
	      terminal->data->selection[k] =
		(buf5250[subwindowcount][i][j] & A_X5250_CONTENTMASK);
	    }
	  k++;
	}
    }

  /* Prepare the selection */
  if (subwindowcount == 0)
    {
      XSetSelectionOwner (display, XA_PRIMARY, terminal->data->win,
			  report->xbutton.time);
      if (XGetSelectionOwner (display, XA_PRIMARY) != terminal->data->win)
	{
	  fprintf (stderr, "Did not get selection ownership\n");
	  return (button);
	}
    }
  else
    {
      windowlist =
	terminal->data->tn5250display->display_buffers->window_list;
      subwindow =
	tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
      XSetSelectionOwner (display, XA_PRIMARY,
			  subwindow->data->win, report->xbutton.time);
      if (XGetSelectionOwner (display, XA_PRIMARY) != subwindow->data->win)
	{
	  fprintf (stderr, "Did not get selection ownership\n");
	  return (button);
	}
    }

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
      windowlist =
	terminal->data->tn5250display->display_buffers->window_list;
      subwindow =
	tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
      draw_caret (subwindow->data->win,
		  terminal->data->gcyellow,
		  terminal->data->font_info,
		  terminal->data->row, terminal->data->column,
		  terminal->data->havefocus, terminal->data->ruler,
		  subwindowcount);
    }
#ifdef DEBUG
  fprintf (stderr, "%s", terminal->data->selection);
#endif
  return (button);
}
