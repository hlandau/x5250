/*
 * Copyright 2003 James Rich
 *
 * Portions are Copyright 1989 O'Reilly and Associates, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* If getopt.h exists then getopt_long() probably does as well.  If
 * getopt.h doesn't exist (like on Solaris) then we probably need to use
 * plain old getopt().
 */
#ifdef HAVE_GETOPT_H
#define _GNU_SOURCE
#include <getopt.h>
#endif
#include <locale.h>

#include "x5250.h"
#include "x5250term.h"
#include "resources.h"
#include "debug.h"
#include "help.h"


#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))



/* Display and screen_num are used as arguments to nearly every Xlib routine, 
 * so it simplifies routine calls to declare them global.  If there were 
 * additional source files, these variables would be declared extern in
 * them. */
Display *display;
char *display_name = NULL;
int screen_num;
Screen *screen_ptr;



/* pixel values */
unsigned long foreground_pixel, background_pixel, border_pixel;
unsigned long menu_pixel, menuborder_pixel;
unsigned long red_pixel, green_pixel, blue_pixel, black_pixel, white_pixel,
  yellow_pixel, turquoise_pixel, pink_pixel, cursor_pixel;
char red_color[20];
char green_color[20];
char blue_color[20];
char black_color[20];
char white_color[20];
char yellow_color[20];
char turquoise_color[20];
char pink_color[20];
char grey_color[20];

char *progname;
char *sessionname;
int placementx;
int placementy;
int mainwindowwidth;
int mainwindowheight;
char *font;
char *font132;
char *fontui;
Bool separatordots;
Bool debugresources = False;
int globalargc;
char **globalargv;
int clear_range_start_row;
int clear_range_start_column;
int clear_range_end_row;
int clear_range_end_column;
Bool clear_range_flushed = True;



/* The 5250 screen buffer.  This contains an entry for every character
 * on the screen.  Each element consists of the character that should
 * be displayed, the color to draw in, and the font to use.  Each of
 * these attributes can be found be applying the appropriate mask (e.g
 * A_X5250_COLORMASK and A_X5250_BOLDMASK).
 */
unsigned int ***buf5250;

/* The previous 5250 screen buffer.  This contains an entry for every character
 * on the previous screen.  This has the same attributes as buf5250 above.
 * This is used to compare the contents of the current window to the previous
 * buffer.  Where they differ, we erase on the current window.  Note that
 * this is always the last screen shown (whether it is a window or not).
 */
unsigned int prevbuf[27][133] = { {0} };

/* The 5250 field buffer.  This contains an entry for every screen and
 * window.
 */
screenfields *fields5250 = NULL;

/* This is the number of 5250 "windows" */
int subwindowcount = 0;

/* This is the number of columns and rows on the screen.  Two
 * combinations are possible:  80x24 and 132x27.
 */
int screencolumns = 80;
int screenrows = 24;

/* This contains the X11 keysyms to tn5250 key maps */
keymap *kmaplist = NULL;

/* This is the name used by the resource databases */
char *appname = NULL;

/* This is the merged resource database */
XrmDatabase rDB;

/* This get a scaled font name to load */
char *scale_font (int newsize, char *font);

/* This just checks what arguments (options) were passed to x5250 on the
 * command line.
 */
int check_options (Tn5250Config * tn5250config);

/* This draws a string to a window
 */
void
flush_drawable_text (Window win, GC gcgreen, GC gcwhite, GC gcred,
		     GC gcblue, GC gcyellow, GC gcturquoise, GC gcpink,
		     GC gcblack, unsigned int color, int coord_x,
		     int coord_y, unsigned char *string);

/* This calculates an area of a window to clear
 */
void extend_erase_area (Window win, int row, int column);

/* This clears the calculate window area above
 */
void flush_erase_area (Window win, XFontStruct * font_info);



int
main (int argc, char **argv)
{
  const char *tracefile;
  extern char *optarg;
  extern int optind, opterr, optopt;
  char *host = NULL;

  /* These are the lib5250 objects we need */
  Tn5250Config *tn5250config = NULL;
  Tn5250Stream *tn5250stream = NULL;
  Tn5250Display *tn5250display = NULL;
  Tn5250Terminal *tn5250term = NULL;
  Tn5250Session *tn5250session = NULL;
#ifdef MACRO_SUPPORT
  Tn5250Macro *tn5250macro = NULL;
#endif

  setlocale (LC_ALL, "");



  /*
     argv[1] = "test";
     argc++;
   */
  progname = argv[0];
  globalargc = argc;
  globalargv = argv;
  sessionname = NULL;
  font = NULL;



  /* Get the X resources */
  XrmInitialize ();
  parse_command_line_X ();



  /* set up the lib5250 connection */
  tn5250config = tn5250_config_new ();

  if (tn5250_config_load_default (tn5250config) == -1)
    {
      tn5250_config_unref (tn5250config);
      exit (1);
    }




  if (check_options (tn5250config) != 0)
    {
      tn5250_config_unref (tn5250config);
      return (-1);
    }


  /* We probably want to disable this so we can gather up the options that
   * are specific to tn5250.  This way we can support both the newer GNU
   * style long options and the traditional tn5250 +/- style options.
   */
  /*
     if ((optind != argc) && (optind != argc - 1))
     {
     printf ("We crapped!!\n");
     printf ("Unknown argument '%s'\n", argv[optind + 1]);
     usage ();
     return -1;
     }
   */



  /* Set the session name */
  if (sessionname == NULL)
    {
      sessionname =
	malloc (strlen (tn5250_config_get (tn5250config, "host")) + 9);
      sprintf (sessionname, "x5250 - %s",
	       tn5250_config_get (tn5250config, "host"));
      sessionname[strlen (tn5250_config_get (tn5250config, "host")) + 8] =
	'\0';
    }




  tracefile = tn5250_config_get (tn5250config, "trace");
  if (tracefile != NULL)
    {
      tn5250_log_open (tn5250_config_get (tn5250config, "trace"));
    }




  /* Set up the lib5250 stream */
  tn5250stream =
    tn5250_stream_open (tn5250_config_get (tn5250config, "host"),
			tn5250config);
  if (tn5250stream == NULL)
    {
      tn5250_config_unref (tn5250config);
      exit (0);
    }

  tn5250display = tn5250_display_new ();

  if (tn5250_display_config (tn5250display, tn5250config) == -1)
    {
      tn5250_stream_destroy (tn5250stream);
      tn5250_config_unref (tn5250config);
      exit (0);
    }

  tn5250term = x5250_terminal_new ();

  if (tn5250term == NULL)
    {
      tn5250_stream_destroy (tn5250stream);
      tn5250_config_unref (tn5250config);
      exit (0);
    }

  if (tn5250_terminal_config (tn5250term, tn5250config) == -1)
    {
      tn5250_terminal_destroy (tn5250term);
      tn5250_stream_destroy (tn5250stream);
      tn5250_config_unref (tn5250config);
      exit (0);
    }

  tn5250_terminal_init (tn5250term);
  tn5250_display_set_terminal (tn5250display, tn5250term);
  tn5250session = tn5250_session_new ();
  tn5250_display_set_session (tn5250display, tn5250session);
  tn5250term->conn_fd = tn5250_stream_socket_handle (tn5250stream);
  tn5250_session_set_stream (tn5250session, tn5250stream);

  if (tn5250_session_config (tn5250session, tn5250config) == -1)
    {
      tn5250_session_destroy (tn5250session);
      tn5250_terminal_destroy (tn5250term);
      tn5250_stream_destroy (tn5250stream);
      tn5250_config_unref (tn5250config);
      exit (0);
    }

#ifdef MACRO_SUPPORT
  tn5250macro = tn5250_macro_init ();
  tn5250_macro_attach (tn5250display, tn5250macro);
#endif

  /* Put the tn5250display into the terminal object here since the init
   * function doesn't get passed the display object.
   */
  tn5250term->data->tn5250display = tn5250display;

  /* Create the dialog for the macros (this must be done here instead of
   * inside of x5250_terminal_init because the macro structure isn't complete
   * when that runs.
   */
  /* First we need to read the macro file.  Usually this is done by lib5250
   * when the user presses the macro key combination.  We force it here so that
   * the Tn5250Macro structure knows what macros exist.
   */
  macro_loadfile (tn5250macro);
  /* And finally create the dialog. */
  createmacrodialog (tn5250term, tn5250macro);

  tn5250_session_main_loop (tn5250session);

#ifdef MACRO_SUPPORT
  if (tn5250macro != NULL)
    {
      tn5250_macro_exit (tn5250macro);
    }
#endif
  if (tn5250term != NULL)
    {
      tn5250_terminal_term (tn5250term);
    }
  if (tn5250session != NULL)
    {
      tn5250_session_destroy (tn5250session);
    }
  else if (tn5250stream != NULL)
    {
      tn5250_stream_destroy (tn5250stream);
    }
  if (tn5250config != NULL)
    {
      tn5250_config_unref (tn5250config);
    }
  return (0);
}




void
get_GC (Window win, GC * gc, XFontStruct * font_info, int instance)
{
  unsigned long valuemask = 0;	/* ignore XGCvalues and use defaults */
  XGCValues values;
  unsigned int line_width = 0;
  int line_style = LineSolid;
  int cap_style = CapRound;
  int join_style = JoinRound;
  int dash_offset = 0;
  static char dash_list[] = { 1, 1 };
  int list_length = 2;

  /* Create default Graphics Context */
  *gc = XCreateGC (display, win, valuemask, &values);

  /* specify font */
  XSetFont (display, *gc, font_info->fid);

  /* specify black foreground since default may be white on white */
  switch (instance)
    {
    case 1:
      XSetForeground (display, *gc, green_pixel);
      break;
    case 2:
      XSetForeground (display, *gc, white_pixel);
      break;
    case 3:
      XSetForeground (display, *gc, red_pixel);
      break;
    case 4:
      XSetForeground (display, *gc, blue_pixel);
      break;
    case 5:
      XSetForeground (display, *gc, yellow_pixel);
      break;
    case 6:
      XSetForeground (display, *gc, turquoise_pixel);
      break;
    case 7:
      XSetForeground (display, *gc, pink_pixel);
      break;
    case 8:
      XSetForeground (display, *gc, black_pixel);
      break;
    case 9:
      XSetForeground (display, *gc, white_pixel);
      break;
    case 10:
      XSetForeground (display, *gc, black_pixel);
      XSetBackground (display, *gc, menu_pixel);
      break;
    }

  /* set line attributes */
  XSetLineAttributes (display, *gc, line_width, line_style, cap_style,
		      join_style);

  /* set dashes to be line_width in length */
  XSetDashes (display, *gc, dash_offset, dash_list, list_length);
  return;
}




void
load_font (XFontStruct ** font_info, char *font, int pointsize)
{
  char *scaledfont;

  if (is_font_scalable (font))
    {
      scaledfont = scale_font (pointsize, font);

      if ((*font_info = XLoadQueryFont (display, scaledfont)) == NULL)
	{
	  fprintf (stderr, "%s: Cannot open %s font\n", progname, scaledfont);
	  exit (-1);
	}
      free (scaledfont);
    }
  else
    {
      if ((*font_info = XLoadQueryFont (display, font)) == NULL)
	{
	  fprintf (stderr, "%s: Cannot open %s font\n", progname, font);
	  exit (-1);
	}
    }

  return;
}




char *
scale_font (int newsize, char *font)
{
  int i, j, field, length;
  char newfont[500];
  char *newfontname;
  int resx, resy;

  if (!is_font_scalable (font))
    {
      return NULL;
    }

  /*
     fprintf (stderr, "Performing font scaling.  Starting font:\n%s\n", font);
   */

  /* Calculate screen resolution in dots per inch (25.4mm = 1 inch) */
  resx =
    DisplayWidth (display,
		  screen_num) / (DisplayWidthMM (display, screen_num) / 25.4);
  resy =
    DisplayHeight (display,
		   screen_num) / (DisplayHeightMM (display,
						   screen_num) / 25.4);

  length = strlen (font);

  for (i = j = field = 0;
       (font[i] != '\0') && (field <= 14) && (i < length) && (i < 500); i++)
    {
      newfont[j++] = font[i];

      if (font[i] == '-')
	{
	  field++;

	  switch (field)
	    {
	    case 7:		/* pixel size */
	    case 12:		/* average width */
	      newfont[j] = '*';
	      j++;

	      if (font[i + 1] != '\0')
		{
		  i++;
		}

	      break;
	    case 8:		/* point size */
	      sprintf (&newfont[j], "%d", newsize);

	      while (newfont[j] != '\0')
		{
		  j++;
		}

	      if (font[i + 1] != '\0')
		{
		  i++;
		}

	      break;
	    case 9:		/* x resolution */
	    case 10:		/* y resolution */
	      sprintf (&newfont[j], "%d", (field == 9) ? resx : resy);

	      while (newfont[j] != '\0')
		{
		  j++;
		}

	      while ((font[i + 1] != '-') && (font[i + 1] != '\0'))
		{
		  i++;
		}

	      break;
	    }
	}
    }

  newfont[j] = '\0';
  length = strlen (newfont);

  /* Fail if the resulting font name is obviously wrong. */
  if ((length < 14) || (length >= 498))
    {
      return NULL;
    }

  newfontname = malloc ((strlen (newfont) + 1) * sizeof (char));
  strncpy (newfontname, newfont, strlen (newfont) + 1);
  /*
     fprintf (stderr, "Using font:\n%s\nDone font scaling\n", newfontname);
   */
  return newfontname;
}




void
place_text (Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
	    GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
	    XFontStruct * font_info, unsigned int win_width,
	    unsigned int win_height, int bufferid)
{
  int widthbuf;
  int font_height;
  int i, j;
  unsigned int colorattrib;
  unsigned int fontattrib;
  unsigned int oldcolor;
  unsigned int oldfont;
  unsigned char string[133];
  int idx;
  int startx;
  int do_attributes;
  /*
     int boldactive;
   */
  int reverseactive;

  do_attributes = 0;
  colorattrib = A_X5250_GREEN;
  oldcolor = A_X5250_GREEN;
  fontattrib = A_X5250_NORMAL;
  oldfont = A_X5250_NORMAL;
  font_height = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  widthbuf = font_info->max_bounds.width;
  idx = 0;
  memset (string, '\0', sizeof (string));
  startx = -1;

  /* Since we can draw things in any order, we'll draw everything in one
   * graphics context, then switch to the next gc and draw everything in
   * that one, etc.  Start with normal font, then do bold, changing the gc
   * in the process.
   */
  for (i = 0; i < screenrows; i++)
    {
      if (clear_range_flushed == False)
	{
	  flush_erase_area (win, font_info);
	}
      if (strlen (string) != 0)
	{
	  flush_drawable_text (win, gcgreen, gcwhite, gcred,
			       gcblue, gcyellow, gcturquoise,
			       gcpink, gcblack, oldcolor,
			       1 + (widthbuf * startx),
			       ((font_height + ROWSPACING) * (i - 1)) +
			       BASELINESPACE + 3 +
			       font_info->max_bounds.ascent, string);
	  startx = -1;
	  memset (string, '\0', sizeof (string));
	  idx = 0;
	}
      for (j = 0; j <= screencolumns; j++)
	{
	  /* If the current position differs from whatever was in this same
	   * position previously, add this position to the area to be cleared.
	   * Also force position 0,0 to be cleared to get rid of any caret
	   * that may have been there.  This really should probably be fixed
	   * so that erase_caret() handles it but this is a quickie fix.
	   * FIXME
	   */
	  if ((buf5250[bufferid][i][j] != prevbuf[i][j])
	      || ((i == 0) && j == 0))
	    {
#ifdef DEBUG
	      fprintf (stderr, "%c", ' ');
#endif
	      extend_erase_area (win, i, j);
	    }
	  else
	    {
	      if (clear_range_flushed == False)
		{
		  flush_erase_area (win, font_info);
		}
#ifdef DEBUG
	      fprintf (stderr, "%c", buf5250[bufferid][i][j]);
#endif
	    }
	  prevbuf[i][j] = buf5250[bufferid][i][j];

	  if (((buf5250[bufferid][i][j] & A_X5250_CONTENTMASK) != 0)
	      && ((buf5250[bufferid][i][j] & A_X5250_NONDISPLAYMASK) == 0))
	    {
	      colorattrib = buf5250[bufferid][i][j] & A_X5250_COLORMASK;
	      fontattrib = buf5250[bufferid][i][j] & A_X5250_FONTMASK;

	      if ((startx == -1) && (fontattrib == A_X5250_NORMAL))
		{
		  startx = j;
		}

	      if (colorattrib != oldcolor)
		{
		  if (strlen (string) != 0)
		    {
		      if (clear_range_flushed == False)
			{
			  flush_erase_area (win, font_info);
			}
		      flush_drawable_text (win, gcgreen, gcwhite, gcred,
					   gcblue, gcyellow, gcturquoise,
					   gcpink, gcblack, oldcolor,
					   1 + (widthbuf * startx),
					   ((font_height + ROWSPACING) * i) +
					   BASELINESPACE + 3 +
					   font_info->max_bounds.ascent,
					   string);
		      startx = -1;
		      memset (string, '\0', sizeof (string));
		      idx = 0;
		    }
		}

	      oldcolor = colorattrib;

	      if (fontattrib == A_X5250_NORMAL)
		{
		  string[idx] = buf5250[bufferid][i][j] & A_X5250_CONTENTMASK;
		  idx++;
		  if (idx > 132)
		    {
		      idx = 132;
		    }
		  string[idx] = '\0';
		}
	      else
		{
		  /*
		     if ((fontattrib & A_X5250_BOLDMASK) == A_X5250_BOLD)
		     {
		     do_attributes = 1;
		     }
		   */
		  if ((fontattrib & A_X5250_REVERSEMASK) == A_X5250_REVERSE)
		    {
		      do_attributes = 1;
		    }
		}
	    }

	  if ((strlen (string) != 0)
	      && ((buf5250[bufferid][i][j] & A_X5250_CONTENTMASK) == 0))
	    {
	      if (clear_range_flushed == False)
		{
		  flush_erase_area (win, font_info);
		}
	      flush_drawable_text (win, gcgreen, gcwhite, gcred,
				   gcblue, gcyellow, gcturquoise,
				   gcpink, gcblack, oldcolor,
				   1 + (widthbuf * startx),
				   ((font_height + ROWSPACING) * i) +
				   BASELINESPACE + 3 +
				   font_info->max_bounds.ascent, string);
	      startx = -1;
	      memset (string, '\0', sizeof (string));
	      idx = 0;
	    }
	}
    }

  /* Now change the graphics context to bold font and redo the buffer
   * for bold characters.
   */
  if (do_attributes)
    {
      /*
         boldactive = 0;
       */
      reverseactive = 0;
      idx = 0;
      memset (string, '\0', sizeof (string));
      startx = -1;

      for (i = 0; i < screenrows; i++)
	{
	  if (strlen (string) != 0)
	    {
	      if (clear_range_flushed == False)
		{
		  flush_erase_area (win, font_info);
		}
	      flush_drawable_text (win, gcgreen, gcwhite, gcred,
				   gcblue, gcyellow, gcturquoise,
				   gcpink, gcblack, oldcolor,
				   1 + (widthbuf * startx),
				   ((font_height + ROWSPACING) * i) +
				   BASELINESPACE + 3 +
				   font_info->max_bounds.ascent, string);
	      startx = -1;
	      memset (string, '\0', sizeof (string));
	      idx = 0;
	    }
	  for (j = 0; j <= screencolumns; j++)
	    {
	      /*
	         if (buf5250[bufferid][i][j] == 0)
	         {
	         buf5250[bufferid][i][j] = '@';
	         }
	       */
	      if (((buf5250[bufferid][i][j] & A_X5250_CONTENTMASK) != 0)
		  && ((buf5250[bufferid][i][j] & A_X5250_NONDISPLAYMASK) ==
		      0))
		{
		  colorattrib = buf5250[bufferid][i][j] & A_X5250_COLORMASK;
		  fontattrib = buf5250[bufferid][i][j] & A_X5250_FONTMASK;

		  if ((startx == -1) && (fontattrib != A_X5250_NORMAL))
		    {
		      startx = j;
		    }

		  if ((colorattrib != oldcolor) || (fontattrib != oldfont))
		    {
		      if (strlen (string) != 0)
			{
			  if (clear_range_flushed == False)
			    {
			      flush_erase_area (win, font_info);
			    }
			  flush_drawable_text (win, gcgreen, gcwhite, gcred,
					       gcblue, gcyellow, gcturquoise,
					       gcpink, gcblack, oldcolor,
					       1 + (widthbuf * startx),
					       ((font_height +
						 ROWSPACING) * i) +
					       BASELINESPACE + 3 +
					       font_info->max_bounds.ascent,
					       string);
			  startx = -1;
			  memset (string, '\0', sizeof (string));
			  idx = 0;
			}
		    }

		  oldcolor = colorattrib;
		  oldfont = fontattrib;

		  if (fontattrib != A_X5250_NORMAL)
		    {
		      string[idx] =
			buf5250[bufferid][i][j] & A_X5250_CONTENTMASK;
		      idx++;
		      if (idx > 132)
			{
			  idx = 132;
			}
		      string[idx] = '\0';
		    }

		  /*
		     if (((fontattrib & A_X5250_BOLDMASK) == A_X5250_BOLD)
		     && boldactive == 0)
		     {
		     boldactive = 1;
		     load_font (&font_info, 100, 1);
		     XSetFont (display, gcgreen, font_info->fid);
		     XSetFont (display, gcwhite, font_info->fid);
		     XSetFont (display, gcred, font_info->fid);
		     XSetFont (display, gcblue, font_info->fid);
		     XSetFont (display, gcyellow, font_info->fid);
		     XSetFont (display, gcturquoise, font_info->fid);
		     XSetFont (display, gcpink, font_info->fid);
		     XSetFont (display, gcblack, font_info->fid);
		     }

		     if (((fontattrib & A_X5250_BOLDMASK) != A_X5250_BOLD)
		     && boldactive == 1)
		     {
		     boldactive = 0;
		     load_font (&font_info, 100, 0);
		     XSetFont (display, gcgreen, font_info->fid);
		     XSetFont (display, gcwhite, font_info->fid);
		     XSetFont (display, gcred, font_info->fid);
		     XSetFont (display, gcblue, font_info->fid);
		     XSetFont (display, gcyellow, font_info->fid);
		     XSetFont (display, gcturquoise, font_info->fid);
		     XSetFont (display, gcpink, font_info->fid);
		     XSetFont (display, gcblack, font_info->fid);
		     }
		   */

		  if (((fontattrib & A_X5250_REVERSEMASK) == A_X5250_REVERSE)
		      && reverseactive == 0)
		    {
		      reverseactive = 1;
		      XSetForeground (display, gcgreen, background_pixel);
		      XSetBackground (display, gcgreen, green_pixel);
		      XSetForeground (display, gcwhite, background_pixel);
		      XSetBackground (display, gcwhite, white_pixel);
		      XSetForeground (display, gcred, background_pixel);
		      XSetBackground (display, gcred, red_pixel);
		      XSetForeground (display, gcblue, background_pixel);
		      XSetBackground (display, gcblue, blue_pixel);
		      XSetForeground (display, gcyellow, background_pixel);
		      XSetBackground (display, gcyellow, yellow_pixel);
		      XSetForeground (display, gcturquoise, background_pixel);
		      XSetBackground (display, gcturquoise, turquoise_pixel);
		      XSetForeground (display, gcpink, background_pixel);
		      XSetBackground (display, gcpink, pink_pixel);
		      XSetForeground (display, gcblack, background_pixel);
		      XSetBackground (display, gcblack, black_pixel);
		    }

		  if (((fontattrib & A_X5250_REVERSEMASK) != A_X5250_REVERSE)
		      && reverseactive == 1)
		    {
		      reverseactive = 0;
		      XSetBackground (display, gcgreen, background_pixel);
		      XSetForeground (display, gcgreen, green_pixel);
		      XSetBackground (display, gcwhite, background_pixel);
		      XSetForeground (display, gcwhite, white_pixel);
		      XSetBackground (display, gcred, background_pixel);
		      XSetForeground (display, gcred, red_pixel);
		      XSetBackground (display, gcblue, background_pixel);
		      XSetForeground (display, gcblue, blue_pixel);
		      XSetBackground (display, gcyellow, background_pixel);
		      XSetForeground (display, gcyellow, yellow_pixel);
		      XSetBackground (display, gcturquoise, background_pixel);
		      XSetForeground (display, gcturquoise, turquoise_pixel);
		      XSetBackground (display, gcpink, background_pixel);
		      XSetForeground (display, gcpink, pink_pixel);
		      XSetBackground (display, gcblack, background_pixel);
		      XSetForeground (display, gcblack, black_pixel);
		    }
		}

	      if ((strlen (string) != 0)
		  && ((buf5250[bufferid][i][j] & A_X5250_CONTENTMASK) == 0))
		{
		  if (clear_range_flushed == False)
		    {
		      flush_erase_area (win, font_info);
		    }
		  flush_drawable_text (win, gcgreen, gcwhite, gcred,
				       gcblue, gcyellow, gcturquoise,
				       gcpink, gcblack, oldcolor,
				       1 + (widthbuf * startx),
				       ((font_height + ROWSPACING) * i) +
				       BASELINESPACE + 3 +
				       font_info->max_bounds.ascent, string);
		  startx = -1;
		  memset (string, '\0', sizeof (string));
		  idx = 0;
		}
	    }
	}
    }

  /*
     if (boldactive)
     {
     load_font (&font_info, 100, 0);
     XSetFont (display, gcgreen, font_info->fid);
     XSetFont (display, gcwhite, font_info->fid);
     XSetFont (display, gcred, font_info->fid);
     XSetFont (display, gcblue, font_info->fid);
     XSetFont (display, gcyellow, font_info->fid);
     XSetFont (display, gcturquoise, font_info->fid);
     XSetFont (display, gcpink, font_info->fid);
     XSetFont (display, gcblack, font_info->fid);
     }
   */

  if (reverseactive)
    {
      XSetBackground (display, gcgreen, background_pixel);
      XSetForeground (display, gcgreen, green_pixel);
      XSetBackground (display, gcwhite, background_pixel);
      XSetForeground (display, gcwhite, white_pixel);
      XSetBackground (display, gcred, background_pixel);
      XSetForeground (display, gcred, red_pixel);
      XSetBackground (display, gcblue, background_pixel);
      XSetForeground (display, gcblue, blue_pixel);
      XSetBackground (display, gcyellow, background_pixel);
      XSetForeground (display, gcyellow, yellow_pixel);
      XSetBackground (display, gcturquoise, background_pixel);
      XSetForeground (display, gcturquoise, turquoise_pixel);
      XSetBackground (display, gcpink, background_pixel);
      XSetForeground (display, gcpink, pink_pixel);
      XSetBackground (display, gcblack, background_pixel);
      XSetForeground (display, gcblack, black_pixel);
    }
  return;
}




void
place_character (Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
		 GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
		 XFontStruct * font_info, int row, int column, int bufferid,
		 unsigned int character)
{
  int widthbuf;
  int font_height;
  unsigned int colorattrib;
  /*
     unsigned int fontattrib;
   */
  unsigned int reverseattrib;
  unsigned char c;
  char ch[2];

  font_height = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  widthbuf = font_info->max_bounds.width;
  prevbuf[row][column] = character;

  if ((character & A_X5250_CONTENTMASK) != 0)
    {
      if ((character & A_X5250_NONDISPLAYMASK) != 0)
	{
	  c = ' ';
	}
      else
	{
	  c = character & A_X5250_CONTENTMASK;
	}
      ch[0] = c;
      ch[1] = '\0';
      colorattrib = character & A_X5250_COLORMASK;
      /*
         fontattrib = character & A_X5250_BOLDMASK;
       */
      reverseattrib = character & A_X5250_REVERSEMASK;

      /*
         if (fontattrib == A_X5250_BOLD)
         {
         load_font (&font_info, 100, 1);
         XSetFont (display, gcgreen, font_info->fid);
         XSetFont (display, gcwhite, font_info->fid);
         XSetFont (display, gcred, font_info->fid);
         XSetFont (display, gcblue, font_info->fid);
         XSetFont (display, gcyellow, font_info->fid);
         XSetFont (display, gcturquoise, font_info->fid);
         XSetFont (display, gcpink, font_info->fid);
         XSetFont (display, gcblack, font_info->fid);
         }
       */

      if (reverseattrib == A_X5250_REVERSE)
	{
	  XSetForeground (display, gcgreen, background_pixel);
	  XSetBackground (display, gcgreen, green_pixel);
	  XSetForeground (display, gcwhite, background_pixel);
	  XSetBackground (display, gcwhite, white_pixel);
	  XSetForeground (display, gcred, background_pixel);
	  XSetBackground (display, gcred, red_pixel);
	  XSetForeground (display, gcblue, background_pixel);
	  XSetBackground (display, gcblue, blue_pixel);
	  XSetForeground (display, gcyellow, background_pixel);
	  XSetBackground (display, gcyellow, yellow_pixel);
	  XSetForeground (display, gcturquoise, background_pixel);
	  XSetBackground (display, gcturquoise, turquoise_pixel);
	  XSetForeground (display, gcpink, background_pixel);
	  XSetBackground (display, gcpink, pink_pixel);
	  XSetForeground (display, gcblack, background_pixel);
	  XSetBackground (display, gcblack, black_pixel);
	}

      switch (colorattrib)
	{
	case A_X5250_GREEN:
	  XDrawImageString (display, win, gcgreen, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_WHITE:
	  XDrawImageString (display, win, gcwhite, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_RED:
	  XDrawImageString (display, win, gcred, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_BLUE:
	  XDrawImageString (display, win, gcblue, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_YELLOW:
	  XDrawImageString (display, win, gcyellow, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_TURQUOISE:
	  XDrawImageString (display, win, gcturquoise,
			    1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_PINK:
	  XDrawImageString (display, win, gcpink, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	case A_X5250_BLACK:
	  XDrawImageString (display, win, gcblack, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  break;
	}

      /*
         if (fontattrib == A_X5250_BOLD)
         {
         load_font (&font_info, 100, 0);
         XSetFont (display, gcgreen, font_info->fid);
         XSetFont (display, gcwhite, font_info->fid);
         XSetFont (display, gcred, font_info->fid);
         XSetFont (display, gcblue, font_info->fid);
         XSetFont (display, gcyellow, font_info->fid);
         XSetFont (display, gcturquoise, font_info->fid);
         XSetFont (display, gcpink, font_info->fid);
         XSetFont (display, gcblack, font_info->fid);
         }
       */

      if (reverseattrib == A_X5250_REVERSE)
	{
	  XSetBackground (display, gcgreen, background_pixel);
	  XSetForeground (display, gcgreen, green_pixel);
	  XSetBackground (display, gcwhite, background_pixel);
	  XSetForeground (display, gcwhite, white_pixel);
	  XSetBackground (display, gcred, background_pixel);
	  XSetForeground (display, gcred, red_pixel);
	  XSetBackground (display, gcblue, background_pixel);
	  XSetForeground (display, gcblue, blue_pixel);
	  XSetBackground (display, gcyellow, background_pixel);
	  XSetForeground (display, gcyellow, yellow_pixel);
	  XSetBackground (display, gcturquoise, background_pixel);
	  XSetForeground (display, gcturquoise, turquoise_pixel);
	  XSetBackground (display, gcpink, background_pixel);
	  XSetForeground (display, gcpink, pink_pixel);
	  XSetBackground (display, gcblack, background_pixel);
	  XSetForeground (display, gcblack, black_pixel);
	}
    }
  return;
}




void
place_graphics (Tn5250Terminal * terminal, Window win, int bufferid)
{
  int font_height;
  int widthbuf;
  int i, j, k;
  unsigned int colorattrib;
  unsigned int underlineattrib;
  int underlineactive;
  int underlinecount;
  int underlinestart;
  unsigned int columnsepattrib;
  int columnsepactive;
  int columnsepcount;
  int columnsepstart;
  int columnsepheight;

  font_height = terminal->data->font_info->max_bounds.ascent +
    terminal->data->font_info->max_bounds.descent;
  widthbuf = terminal->data->font_info->max_bounds.width;

  if (separatordots == True)
    {
      columnsepheight = 1;
    }
  else
    {
      columnsepheight = font_height;
    }


#ifdef DEBUG
  /* Draw the underlines for the input fields */
  for (i = 0; i < fields5250[bufferid].totalfields; i++)
    {
      XDrawLine (display, win, terminal->data->gcgreen,
		 widthbuf * fields5250[bufferid].fields[i].column,
		 ((font_height +
		   ROWSPACING) * (fields5250[bufferid].fields[i].row +
				  1)) + BASELINESPACE,
		 (widthbuf * fields5250[bufferid].fields[i].column) +
		 (widthbuf * fields5250[bufferid].fields[i].length),
		 ((font_height +
		   ROWSPACING) * (fields5250[bufferid].fields[i].row +
				  1)) + BASELINESPACE);
    }
#endif

  underlineactive = 0;
  underlinecount = 0;
  columnsepactive = 0;
  columnsepcount = 0;
  for (i = 0; i < screenrows; i++)
    {
      for (j = 0; j <= screencolumns; j++)
	{
	  underlineattrib = buf5250[bufferid][i][j] & A_X5250_UNDERLINEMASK;
	  columnsepattrib = buf5250[bufferid][i][j] & A_X5250_COLUMNSEPMASK;

	  if ((underlineattrib & A_X5250_UNDERLINEMASK) == A_X5250_UNDERLINE)
	    {
	      colorattrib = buf5250[bufferid][i][j] & A_X5250_COLORMASK;

	      if (!underlineactive)
		{
		  underlinestart = j;
		}
	      underlinecount++;
	      underlineactive = 1;
	    }
	  else
	    {
	      if (underlineactive)
		{
		  switch (colorattrib)
		    {
		    case A_X5250_GREEN:
		      XDrawLine (display, win, terminal->data->gcgreen,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_WHITE:
		      XDrawLine (display, win, terminal->data->gcwhite,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_RED:
		      XDrawLine (display, win, terminal->data->gcred,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_BLUE:
		      XDrawLine (display, win, terminal->data->gcblue,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_YELLOW:
		      XDrawLine (display, win, terminal->data->gcyellow,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_TURQUOISE:
		      XDrawLine (display, win, terminal->data->gcturquoise,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_PINK:
		      XDrawLine (display, win, terminal->data->gcpink,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    case A_X5250_BLACK:
		      XDrawLine (display, win, terminal->data->gcblack,
				 (widthbuf * underlinestart),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE,
				 (widthbuf * j),
				 ((font_height + ROWSPACING) * (i + 1)) +
				 BASELINESPACE);
		      break;
		    }
		  underlineactive = 0;
		}
	      underlinecount = 0;
	    }

	  if (((columnsepattrib & A_X5250_COLUMNSEPMASK) == A_X5250_COLUMNSEP)
	      && (terminal->data->columnseparators == True))
	    {
	      colorattrib = buf5250[bufferid][i][j] & A_X5250_COLORMASK;

	      if (!columnsepactive)
		{
		  columnsepstart = j;
		}
	      columnsepcount++;
	      columnsepactive = 1;
	    }
	  else
	    {
	      if (columnsepactive)
		{
		  for (k = 0; k <= j - columnsepstart; k++)
		    {
		      switch (colorattrib)
			{
			case A_X5250_GREEN:
			  XDrawLine (display, win, terminal->data->gcgreen,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_WHITE:
			  XDrawLine (display, win, terminal->data->gcwhite,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_RED:
			  XDrawLine (display, win, terminal->data->gcred,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_BLUE:
			  XDrawLine (display, win, terminal->data->gcblue,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_YELLOW:
			  XDrawLine (display, win, terminal->data->gcyellow,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_TURQUOISE:
			  XDrawLine (display, win,
				     terminal->data->gcturquoise,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_PINK:
			  XDrawLine (display, win, terminal->data->gcpink,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			case A_X5250_BLACK:
			  XDrawLine (display, win, terminal->data->gcblack,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE,
				     (widthbuf * (columnsepstart + k)),
				     ((font_height + ROWSPACING) * (i + 1)) +
				     BASELINESPACE - columnsepheight);
			  break;
			}
		    }
		  columnsepactive = 0;
		}
	      columnsepcount = 0;
	    }
	}
    }
  return;
}




void
TooSmall (Window win, GC gc, XFontStruct * font_info)
{
  char *string1 = "Too Small";
  int y_offset, x_offset;

  y_offset = font_info->max_bounds.ascent + 2;
  x_offset = 2;

  /* output text, centered on each line */
  XDrawString (display, win, gc, x_offset, y_offset, string1,
	       strlen (string1));
}




/* Draw a caret (cursor) */
void
draw_caret (Window win, GC gc, XFontStruct * font_info, int row, int column,
	    Bool havefocus, Bool ruler, int bufferid)
{
  int line_style;
  int cap_style = CapRound;
  int join_style = JoinRound;
  int font_height;
  int widthbuf;
  unsigned char c;
  char ch[2];

  /* force a little sanity checking */
  if (row < 0)
    {
#ifdef DEBUG
      fprintf (stderr, "draw_caret(): row is negative!  row: %d", row);
#endif
      row = 0;
    }
  if (column < 0)
    {
#ifdef DEBUG
      fprintf (stderr, "draw_caret(): column is negative!  row: %d", column);
#endif
      column = 0;
    }

  font_height = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  widthbuf = font_info->max_bounds.width;
  XDrawRectangle (display, win, gc,
		  (widthbuf * column) + 1,
		  ((font_height + ROWSPACING) * row) + BASELINESPACE + 3,
		  widthbuf - 1, font_height - 1);

  if (havefocus == True)
    {
      if (buf5250[bufferid][row][column] == 0)
	{
	  XFillRectangle (display, win, gc,
			  (widthbuf * column) + 1,
			  ((font_height + ROWSPACING) * row) + BASELINESPACE +
			  3, widthbuf - 1, font_height - 1);
	}
      else
	{
	  if ((buf5250[bufferid][row][column] & A_X5250_NONDISPLAYMASK) != 0)
	    {
	      c = ' ';
	    }
	  else
	    {
	      c = buf5250[bufferid][row][column] & A_X5250_CONTENTMASK;
	    }
	  ch[0] = c;
	  ch[1] = '\0';
	  XSetForeground (display, gc, background_pixel);
	  XSetBackground (display, gc, cursor_pixel);
	  XDrawImageString (display, win, gc, 1 + (widthbuf * column),
			    ((font_height + ROWSPACING) * row) +
			    BASELINESPACE + 3 + font_info->max_bounds.ascent,
			    ch, 1);
	  XSetForeground (display, gc, cursor_pixel);
	  XSetBackground (display, gc, background_pixel);
	}
    }

  if (ruler == True)
    {
      line_style = LineOnOffDash;
      XSetLineAttributes (display, gc, 0, line_style, cap_style, join_style);
      XDrawLine (display, win, gc, 0,
		 ((font_height + ROWSPACING) * (row + 1)) + BASELINESPACE,
		 (widthbuf * screencolumns) - 1,
		 ((font_height + ROWSPACING) * (row + 1)) + BASELINESPACE);
      XDrawLine (display, win, gc, (widthbuf * column) + 1, 0,
		 (widthbuf * column) + 1,
		 (font_height + ROWSPACING) * screenrows);
      line_style = LineSolid;
      XSetLineAttributes (display, gc, 0, line_style, cap_style, join_style);
    }
  return;
}




/* This erases the caret */
void
erase_caret (Tn5250Terminal * terminal,
	     Window win, GC gcgreen, GC gcwhite, GC gcred, GC gcblue,
	     GC gcyellow, GC gcturquoise, GC gcpink, GC gcblack,
	     XFontStruct * font_info, int row, int column, Bool ruler,
	     int bufferid)
{
  int font_height;
  int widthbuf;
  int i;

  /* Check to see if we were passed garbage.  For some reason
   * x5250_terminal_update() sometimes calculates the incorrect row and
   * column to erase (oldrow - windowrowoffset and oldcolumn -
   * windowcolumnoffset).  The offset is greater than the respective row
   * and column.  I don't know why, but this fixes it.
   *
   * This is almost certainly happening when the caret in previous window
   * was outside the area covered by the next window.  Thus if a new window
   * doesn't cover the old caret, the calculation above would try to erase
   * something outside of the new window.
   *
   * FIXME
   */
  if ((bufferid < 0) || (row < 0) || (column < 0))
    {
      return;
    }

  font_height = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  widthbuf = font_info->max_bounds.width;
  XClearArea (display, win, (widthbuf * column) + 1,
	      ((font_height + ROWSPACING) * row) + BASELINESPACE + 3,
	      widthbuf, font_height, False);

  if (buf5250[bufferid][row][column] != 0)
    {
      place_character (win, gcgreen, gcwhite, gcred, gcblue, gcyellow,
		       gcturquoise, gcpink, gcblack, font_info, row, column,
		       bufferid, buf5250[bufferid][row][column]);
    }

  if (ruler == True)
    {
      XClearArea (display, win, 0,
		  ((font_height + ROWSPACING) * (row + 1)) + BASELINESPACE,
		  (widthbuf * screencolumns), 1, False);
      XClearArea (display, win, (widthbuf * column) + 1, 0, 1,
		  ((font_height + ROWSPACING) * screenrows) + 1, False);
      place_graphics (terminal, win, bufferid);
      for (i = 0; i < screenrows; i++)
	{
	  if (buf5250[bufferid][i][column] != 0)
	    {
	      place_character (win, gcgreen, gcwhite, gcred, gcblue, gcyellow,
			       gcturquoise, gcpink, gcblack, font_info, i,
			       column, bufferid,
			       buf5250[bufferid][i][column]);
	    }
	}
    }
  return;
}





void
paste_text (Window win, Atom property, Tn5250Terminal * terminal)
{
  Tn5250Window *windowlist, *subwindow;
  Atom type_returned;
  int format_returned = 0;
  unsigned long items_returned;
  unsigned long bytes_remaining;
  unsigned char *data;
  int row, column;
  int fieldid;
  int i;
  unsigned char c;

  items_returned = 1024;
  bytes_remaining = 0;

  if ((XGetWindowProperty
       (display, win, property, 0, items_returned, True, AnyPropertyType,
	&type_returned, &format_returned, &items_returned, &bytes_remaining,
	&data) == Success) && (items_returned > 0))
    {
#ifdef DEBUG
      fprintf (stderr, "got the selection:\n[%s]\n", data);
#endif
      if (subwindowcount == 0)
	{
	  erase_caret (terminal, terminal->data->win, terminal->data->gcgreen,
		       terminal->data->gcwhite, terminal->data->gcred,
		       terminal->data->gcblue, terminal->data->gcyellow,
		       terminal->data->gcturquoise, terminal->data->gcpink,
		       terminal->data->gcblack, terminal->data->font_info,
		       terminal->data->row, terminal->data->column,
		       terminal->data->ruler, subwindowcount);
	}
      else
	{
	  windowlist =
	    terminal->data->tn5250display->display_buffers->window_list;
	  subwindow =
	    tn5250_window_list_find_by_id (windowlist, subwindowcount - 1);
	  erase_caret (terminal, subwindow->data->win,
		       terminal->data->gcgreen, terminal->data->gcwhite,
		       terminal->data->gcred, terminal->data->gcblue,
		       terminal->data->gcyellow, terminal->data->gcturquoise,
		       terminal->data->gcpink, terminal->data->gcblack,
		       terminal->data->font_info, terminal->data->row,
		       terminal->data->column, terminal->data->ruler,
		       subwindowcount);
	}

      while (bytes_remaining > 0)
	{
	  if (XGetWindowProperty
	      (display, win, property, 0, items_returned, True,
	       AnyPropertyType, &type_returned, &format_returned,
	       &items_returned, &bytes_remaining, &data) == Success)
	    {
#ifdef DEBUG
	      fprintf (stderr, "got the selection:\n[%s]\n", data);
#endif
	    }
	}

      row = terminal->data->pasterow;
      column = terminal->data->pastecolumn;
      fieldid = terminal->data->fieldid;

      for (i = 0; i < strlen (data); i++)
	{
	  if (data[i] == '\n')
	    {
	      if (terminal->data->alignpaste == True)
		{
		  tn5250_display_do_key (terminal->data->tn5250display,
					 K_NEWLINE);
		  row =
		    tn5250_display_cursor_y (terminal->data->tn5250display);
		  tn5250_dbuffer_cursor_set (terminal->data->
					     tn5250display->display_buffers,
					     row,
					     terminal->data->pastecolumn);
		}
	      else
		{
		  tn5250_display_do_key (terminal->data->tn5250display,
					 K_NEWLINE);
		}
	    }
	  else
	    {
	      tn5250_display_do_key (terminal->data->tn5250display, data[i]);
	      c =
		tn5250_display_char_at (terminal->data->tn5250display, row,
					column);
	      /* UNPRINTABLE -- print block */
	      if ((c == 0x1f) || (c == 0x3F))
		{
		  buf5250[subwindowcount][row][column] = 0;
		}
	      /* UNPRINTABLE -- print blank */
	      else if ((c < 0x40 && c > 0x00) || c == 0xff)
		{
		  buf5250[subwindowcount][row][column] = 0;
		}
	      else
		{
		  c =
		    tn5250_char_map_to_local (tn5250_display_char_map
					      (terminal->data->tn5250display),
					      c);
		  buf5250[subwindowcount][row][column] =
		    c | (fields5250[subwindowcount].fields[fieldid].
			 attributes & 0xffffff00);
		  fields5250[subwindowcount].fields[fieldid].contents[column -
								      fields5250
								      [subwindowcount].fields
								      [fieldid].
								      column]
		    = buf5250[subwindowcount][row][column];
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
				   terminal->data->font_info, row, column,
				   subwindowcount,
				   buf5250[subwindowcount][row][column]);
		}
	      else
		{
		  place_character (subwindow->data->win,
				   terminal->data->gcgreen,
				   terminal->data->gcwhite,
				   terminal->data->gcred,
				   terminal->data->gcblue,
				   terminal->data->gcyellow,
				   terminal->data->gcturquoise,
				   terminal->data->gcpink,
				   terminal->data->gcblack,
				   terminal->data->font_info, row, column,
				   subwindowcount,
				   buf5250[subwindowcount][row][column]);
		}
	    }
	  row = tn5250_display_cursor_y (terminal->data->tn5250display);
	  column = tn5250_display_cursor_x (terminal->data->tn5250display);
	  is_in_field (row, column, &fieldid);
	  terminal->data->row = row;
	  terminal->data->column = column;
	  terminal->data->fieldid = fieldid;
	  /*
	     column++;
	   */
	}
      terminal->data->row = row;
      terminal->data->column = column;
      if (subwindowcount == 0)
	{
	  draw_caret (terminal->data->win,
		      terminal->data->gcyellow,
		      terminal->data->font_info, row, column, True,
		      terminal->data->ruler, subwindowcount);
	}
      else
	{
	  draw_caret (subwindow->data->win, terminal->data->gcyellow,
		      terminal->data->font_info, row, column, True,
		      terminal->data->ruler, subwindowcount);
	}
    }
  return;
}





void
selectionhandler (XEvent * report, Tn5250Terminal * terminal)
{
  XEvent eventsend;

  eventsend.xselection.type = SelectionNotify;

  if (report->xselectionrequest.target != XA_STRING)
    {
      eventsend.xselection.property = None;
#ifdef DEBUG
      fprintf (stderr, "We can't handle the selection request.\n");
#endif
    }
  else
    {
      eventsend.xselection.property = report->xselectionrequest.property;
    }

  XChangeProperty (display, report->xselectionrequest.requestor,
		   report->xselectionrequest.property,
		   report->xselectionrequest.target, 8, PropModeReplace,
		   terminal->data->selection,
		   strlen (terminal->data->selection));
  eventsend.xselection.serial = report->xselectionrequest.serial;
  eventsend.xselection.send_event = report->xselectionrequest.send_event;
  eventsend.xselection.display = report->xselectionrequest.display;
  eventsend.xselection.requestor = report->xselectionrequest.requestor;
  eventsend.xselection.selection = report->xselectionrequest.selection;
  eventsend.xselection.target = report->xselectionrequest.target;
  eventsend.xselection.time = report->xselectionrequest.time;
  XSendEvent (display, report->xselectionrequest.requestor, True, 0,
	      &eventsend);
  return;
}






/* This clears out the buffers before drawing a new screen.
 */
void
destroy_screen (int windowid)
{
  int i, j;

  for (i = 0; i < 27; i++)
    {
      /*
         memset (buf5250[windowid][i], 0, 133);
       */
      for (j = 0; j < 133; j++)
	{
	  buf5250[windowid][i][j] = 0;
	}
    }

  /* Initialize the field structure */
  for (i = 0; i < fields5250[windowid].totalfields; i++)
    {
      fields5250[windowid].fields[i].inputcapable = 0;
      fields5250[windowid].fields[i].continuous = 0;
      fields5250[windowid].fields[i].wordwrap = 0;
      fields5250[windowid].fields[i].attributes = 0;
      fields5250[windowid].fields[i].row = 0;
      fields5250[windowid].fields[i].column = 0;
      fields5250[windowid].fields[i].length = 0;
      fields5250[windowid].fields[i].nextfieldprogressionid = 0;
      memset (fields5250[windowid].fields[i].contents, 0,
	      sizeof (fields5250[windowid].fields[i].contents));
    }
  fields5250[windowid].totalfields = 0;
  return;
}





/* This checks the options x5250 was passed.
 */
int
check_options (Tn5250Config * tn5250config)
{
#ifdef HAVE_GETOPT_H
  struct option options[5];
#endif
  int i;

#ifdef HAVE_GETOPT_H
  options[0].name = "host";
  options[0].has_arg = required_argument;
  options[0].flag = NULL;
  options[0].val = 'h';
  options[1].name = "debug-resources";
  options[1].has_arg = no_argument;
  options[1].flag = NULL;
  options[1].val = 'R';
  options[2].name = "version";
  options[2].has_arg = no_argument;
  options[2].flag = NULL;
  options[2].val = 'v';
  options[3].name = "help";
  options[3].has_arg = no_argument;
  options[3].flag = NULL;
  options[3].val = 'H';
  options[4].name = 0;
  options[4].has_arg = 0;
  options[4].flag = 0;
  options[4].val = 0;
#endif

#ifdef HAVE_GETOPT_H
  while ((i =
	  getopt_long (globalargc, globalargv, "HRvh:", options, NULL)) != -1)
#else
  while ((i = getopt (globalargc, globalargv, "HRvh:")) != -1)
#endif
    {
      switch (i)
	{
	case '?':
	  usage ();
	  return -1;
	case 'h':
	  /*
	     host = optarg;
	   */
	  break;
	case 'R':
	  debugresources = True;
	  break;
	case 'H':
	  print_help ();
	  return -1;
	case 'v':
	  printf ("x5250 version:  %s\n", PACKAGE_VERSION);
	  return -1;
	default:
	  usage ();
	  return -1;
	}
    }

  if (tn5250_config_parse_argv (tn5250config, globalargc, globalargv) == -1)
    {
      usage ();
      return (-1);
    }
  if (tn5250_config_get (tn5250config, "help"))
    {
      usage ();
      return (-1);
    }
  else if (tn5250_config_get (tn5250config, "version"))
    {
      printf ("x5250 version:  %s\n", PACKAGE_VERSION);
      return -1;
    }
  else if (!tn5250_config_get (tn5250config, "host"))
    {
      usage ();
      return (-1);
    }
  return 0;
}





/* Draw a string to the screen.
 */
void
flush_drawable_text (Window win, GC gcgreen, GC gcwhite, GC gcred,
		     GC gcblue, GC gcyellow, GC gcturquoise, GC gcpink,
		     GC gcblack, unsigned int color, int coord_x,
		     int coord_y, unsigned char *string)
{
  switch (color)
    {
    case A_X5250_GREEN:
      XDrawImageString (display, win, gcgreen, coord_x, coord_y, string,
			strlen (string));
      break;
    case A_X5250_WHITE:
      XDrawImageString (display, win, gcwhite, coord_x,
			coord_y, string, strlen (string));
      break;
    case A_X5250_RED:
      XDrawImageString (display, win, gcred, coord_x,
			coord_y, string, strlen (string));
      break;
    case A_X5250_BLUE:
      XDrawImageString (display, win, gcblue, coord_x,
			coord_y, string, strlen (string));
      break;
    case A_X5250_YELLOW:
      XDrawImageString (display, win, gcyellow, coord_x,
			coord_y, string, strlen (string));
      break;
    case A_X5250_TURQUOISE:
      XDrawImageString (display, win, gcturquoise,
			coord_x, coord_y, string, strlen (string));
      break;
    case A_X5250_PINK:
      XDrawImageString (display, win, gcpink, coord_x,
			coord_y, string, strlen (string));
      break;
    case A_X5250_BLACK:
      XDrawImageString (display, win, gcblack, coord_x,
			coord_y, string, strlen (string));
      break;
    }
  return;
}




/* This lets us know if the cursor is currently in a field or not.
 */
int
is_in_field (int row, int column, int *fieldid)
{
  int fieldcheck;

  for (fieldcheck = 0; fieldcheck < fields5250[subwindowcount].totalfields;
       fieldcheck++)
    {
      if (row == fields5250[subwindowcount].fields[fieldcheck].row)
	{
	  if ((column >=
	       fields5250[subwindowcount].fields[fieldcheck].column)
	      && (column <
		  (fields5250[subwindowcount].fields[fieldcheck].column +
		   fields5250[subwindowcount].fields[fieldcheck].length)))
	    {
	      *fieldid = fieldcheck;
	      return (1);
	    }
	}
    }
  return (0);
}




/* This draws the indicator window with the correct system messages/icons.
 */
void
draw_indicator_win (Tn5250Terminal * terminal)
{
  char colrow[7];

  XClearWindow (display, terminal->data->indwin);

  if (terminal->data->ind_message)
    {
      XDrawImageString (display, terminal->data->indwin,
			terminal->data->gcui,
			terminal->data->ui_font_info->max_bounds.width * 23,
			terminal->data->ui_font_info->max_bounds.ascent,
			"MSG", 3);
    }
  if (terminal->data->ind_lock)
    {
      XDrawImageString (display, terminal->data->indwin,
			terminal->data->gcui,
			terminal->data->ui_font_info->max_bounds.width * 9,
			terminal->data->ui_font_info->max_bounds.ascent,
			"X II", 4);
    }
  else if (terminal->data->ind_wait)
    {
      XDrawImageString (display, terminal->data->indwin,
			terminal->data->gcui,
			terminal->data->ui_font_info->max_bounds.width * 9,
			terminal->data->ui_font_info->max_bounds.ascent,
			"X CLOCK", 7);
    }
  else if (terminal->data->ind_busy)
    {
      XDrawImageString (display, terminal->data->indwin,
			terminal->data->gcui,
			terminal->data->ui_font_info->max_bounds.width * 9,
			terminal->data->ui_font_info->max_bounds.ascent,
			"X SYSTEM", 8);
    }
  if (terminal->data->ind_insert)
    {
      XDrawImageString (display, terminal->data->indwin,
			terminal->data->gcui,
			terminal->data->ui_font_info->max_bounds.width * 30,
			terminal->data->ui_font_info->max_bounds.ascent,
			"INSERT", 6);
    }

  sprintf (colrow, "%d/%d", terminal->data->column + 1,
	   terminal->data->row + 1);
  XDrawImageString (display, terminal->data->indwin,
		    terminal->data->gcui,
		    terminal->data->width -
		    (terminal->data->ui_font_info->max_bounds.width * 10),
		    terminal->data->ui_font_info->max_bounds.ascent, colrow,
		    strlen (colrow));
  return;
}





/* This draws any 5250 menus */
void
draw_5250_scrollbar (Tn5250Terminal * terminal)
{
  Tn5250Scrollbar *sbarlist, *sbariter;
  int scrollbarcount =
    terminal->data->tn5250display->display_buffers->scrollbar_count;
  int widthbuf;
  int font_height;

  widthbuf = terminal->data->font_info->max_bounds.width;
  font_height = terminal->data->font_info->max_bounds.ascent +
    terminal->data->font_info->max_bounds.descent;
  sbarlist = terminal->data->tn5250display->display_buffers->scrollbar_list;

  if ((sbariter = sbarlist) != NULL)
    {
      do
	{
	  XDrawRectangle (display, terminal->data->win,
			  terminal->data->gcwhite,
			  sbariter->column * widthbuf,
			  sbariter->row * (font_height + BASELINESPACE),
			  widthbuf,
			  sbariter->size * (font_height + BASELINESPACE));
	  sbariter = sbariter->next;
	}
      while (sbariter != sbarlist);
    }
  return;
}





/* This draws any 5250 menus */
void
draw_5250_menubar (Tn5250Terminal * terminal)
{
  Tn5250Menubar *mbarlist, *mbariter;
  Tn5250Menuitem *itemlist, *itemiter;
  int widthbuf;
  char *itemstring;
  int i;

  widthbuf = terminal->data->font_info->max_bounds.width;
  mbarlist = terminal->data->tn5250display->display_buffers->menubar_list;

  if ((mbariter = mbarlist) != NULL)
    {
      do
	{
	  itemlist = mbariter->menuitem_list;

	  if ((itemiter = itemlist) != NULL)
	    {
	      do
		{
		  itemstring = (char *) malloc (strlen (itemiter->text));
		  for (i = 0; i < strlen (itemiter->text); i++)
		    {
		      itemstring[i] =
			tn5250_char_map_to_local (tn5250_display_char_map
						  (terminal->
						   data->tn5250display),
						  itemiter->text[i]);
		    }

		  if (itemiter->selected)
		    {
		      XSetForeground (display, terminal->data->gcmenu,
				      yellow_pixel);
		    }
		  else if (!itemiter->available)
		    {
		      XSetForeground (display, terminal->data->gcmenu,
				      blue_pixel);
		    }
		  XDrawImageString (display,
				    itemiter->data->win,
				    terminal->data->gcmenu,
				    1 + (widthbuf),
				    BASELINESPACE + 3 +
				    terminal->data->font_info->
				    max_bounds.ascent, itemstring,
				    strlen (itemiter->text));
		  if ((itemiter->selected) || (!itemiter->available))
		    {
		      XSetForeground (display, terminal->data->gcmenu,
				      black_pixel);
		    }
		  free (itemstring);
		  itemiter = itemiter->next;
		}
	      while (itemiter != itemlist);
	    }
	  mbariter = mbariter->next;
	}
      while (mbariter != mbarlist);
    }
  return;
}




/* This calculates an area of a window to clear
 */
void
extend_erase_area (Window win, int row, int column)
{
  static Window oldwin;

  if (win != oldwin)
    {
      clear_range_start_row = row;
      clear_range_start_column = column;
      oldwin = win;
    }

  if (clear_range_flushed == True)
    {
      clear_range_start_row = row;
      clear_range_start_column = column;
    }

  clear_range_end_row = row;
  clear_range_end_column = column;
  clear_range_flushed = False;
  return;
}






/* This clears the calculate window area above
 */
void
flush_erase_area (Window win, XFontStruct * font_info)
{
  int font_height;
  int widthbuf;

#ifdef DEBUG
  printf ("Clearing row %d from %d to %d\n", clear_range_start_row,
	  clear_range_start_column, clear_range_end_column);
#endif

  font_height = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  widthbuf = font_info->max_bounds.width;

  XClearArea (display, win, (widthbuf * clear_range_start_column),
	      ((font_height + ROWSPACING) * clear_range_start_row) +
	      BASELINESPACE + 3,
	      widthbuf * (clear_range_end_column - clear_range_start_column +
			  1) + 1, font_height + BASELINESPACE + 3, False);
  clear_range_flushed = True;
  return;
}
