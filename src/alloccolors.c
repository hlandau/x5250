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
#include "x5250.h"
#include <stdio.h>
#include <stdlib.h>



extern Display *display;
extern int screen_num;
extern Screen *screen_ptr;
extern unsigned long foreground_pixel, background_pixel, border_pixel;
extern unsigned long menu_pixel, menuborder_pixel;
extern unsigned long red_pixel, green_pixel, blue_pixel, black_pixel,
  white_pixel, yellow_pixel, turquoise_pixel, pink_pixel, cursor_pixel;
extern char red_color[20];
extern char green_color[20];
extern char blue_color[20];
extern char black_color[20];
extern char white_color[20];
extern char yellow_color[20];
extern char turquoise_color[20];
extern char pink_color[20];
extern char grey_color[20];

extern char *progname;



#ifdef DEBUG
static char *visual_class[] = {
  "StaticGray",
  "GrayScale",
  "StaticColor",
  "PseudoColor",
  "TrueColor",
  "DirectColor"
};
#endif



int
alloccolors ()
{
  int default_depth;
  Visual *default_visual;
  Colormap default_cmap;
  static char *name[] = {
    "Red", "Green", "Blue", "Black", "White", "Yellow", "Turquoise", "Pink",
    "Grey"
  };
  XColor exact_def;
  int i;
  XVisualInfo visual_info;
  Status result;

  /* Try to allocate colors for PseudoColor, TrueColor, 
   * DirectColor, and StaticColor.  Use black and white
   * for StaticGray and GrayScale */

  default_depth = DefaultDepth (display, screen_num);
  default_visual = DefaultVisual (display, screen_num);
  default_cmap = DefaultColormap (display, screen_num);

  if (default_depth == 1)
    {
      /* must be StaticGray, use black and white */
      border_pixel = WhitePixel (display, screen_num);
      background_pixel = BlackPixel (display, screen_num);
      foreground_pixel = WhitePixel (display, screen_num);
      menu_pixel = WhitePixel (display, screen_num);
      menuborder_pixel = BlackPixel (display, screen_num);
      return (0);
    }

  /* Find the visual class starting at the largest (i=5) and going down */
  for (i = 5; i > 0; i--)
    {
      if (XMatchVisualInfo
	  (display, screen_num, default_depth, i, &visual_info))
	{
	  break;
	}
    }

#ifdef DEBUG
  printf ("%s: found a %s class visual at default_depth.\n", progname,
	  visual_class[i]);
#endif

  if (i < 2)
    {
      /* No color visual available at default_depth.
       * Some applications might call XMatchVisualInfo
       * here to try for a GrayScale visual 
       * if they can use gray to advantage, before 
       * giving up and using black and white.
       */
      border_pixel = BlackPixel (display, screen_num);
      background_pixel = WhitePixel (display, screen_num);
      foreground_pixel = BlackPixel (display, screen_num);
      return (0);
    }

  /* otherwise, got a color visual at default_depth */

  /* The visual we found is not necessarily the 
   * default visual, and therefore it is not necessarily
   * the one we used to create our window.  However,
   * we now know for sure that color is supported, so the
   * following code will work (or fail in a controlled way).
   * Let's check just out of curiosity: */
  if (visual_info.visual != default_visual)
    {
#ifdef DEBUG
      printf
	("%s: PseudoColor visual at default depth is not default visual!\nContinuing anyway...\n",
	 progname);
#endif
    }




  /* Allocate red color */
  if (strlen (red_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", red_color);
#endif
      result = XParseColor (display, default_cmap, red_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[0]);
#endif
      result = XParseColor (display, default_cmap, name[0], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (red_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   red_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[0]);
	}
      red_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  red_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  red_pixel = exact_def.pixel;
	}
    }




  /* Allocate green color.  This is also the foreground color so default to
   * black if it can't be allocated.
   */
  if (strlen (green_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", green_color);
#endif
      result = XParseColor (display, default_cmap, green_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[1]);
#endif
      result = XParseColor (display, default_cmap, name[1], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (green_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   green_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[1]);
	}
      green_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  green_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  green_pixel = exact_def.pixel;
	}
    }




  /* Allocate blue color */
  if (strlen (blue_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", blue_color);
#endif
      result = XParseColor (display, default_cmap, blue_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[2]);
#endif
      result = XParseColor (display, default_cmap, name[2], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (blue_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   blue_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[2]);
	}
      blue_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  blue_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  blue_pixel = exact_def.pixel;
	}
    }




  /* Allocate black color.  This is also the background color so default
   * to white if it can't be allocated.
   */
  if (strlen (black_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", black_color);
#endif
      result = XParseColor (display, default_cmap, black_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[3]);
#endif
      result = XParseColor (display, default_cmap, name[3], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (black_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   black_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[3]);
	}
      black_pixel = WhitePixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  black_pixel = WhitePixel (display, screen_num);
	}
      else
	{
	  black_pixel = exact_def.pixel;
	}
    }




  /* Allocate white color */
  if (strlen (white_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", white_color);
#endif
      result = XParseColor (display, default_cmap, white_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[4]);
#endif
      result = XParseColor (display, default_cmap, name[4], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (white_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   white_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[4]);
	}
      white_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  white_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  white_pixel = exact_def.pixel;
	}
    }




  /* Allocate yellow color */
  if (strlen (yellow_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", yellow_color);
#endif
      result = XParseColor (display, default_cmap, yellow_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[5]);
#endif
      result = XParseColor (display, default_cmap, name[5], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (yellow_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   yellow_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[5]);
	}
      yellow_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  yellow_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  yellow_pixel = exact_def.pixel;
	}
    }




  /* Allocate turquoise color */
  if (strlen (turquoise_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", turquoise_color);
#endif
      result =
	XParseColor (display, default_cmap, turquoise_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[6]);
#endif
      result = XParseColor (display, default_cmap, name[6], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (turquoise_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   turquoise_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[6]);
	}
      turquoise_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  turquoise_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  turquoise_pixel = exact_def.pixel;
	}
    }




  /* Allocate pink color */
  if (strlen (pink_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", pink_color);
#endif
      result = XParseColor (display, default_cmap, pink_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[7]);
#endif
      result = XParseColor (display, default_cmap, name[7], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (pink_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   pink_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[7]);
	}
      pink_pixel = BlackPixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  pink_pixel = BlackPixel (display, screen_num);
	}
      else
	{
	  pink_pixel = exact_def.pixel;
	}
    }




  /* Allocate grey color */
  if (strlen (grey_color) > 0)
    {
#ifdef DEBUG
      printf ("allocating %s\n", grey_color);
#endif
      result = XParseColor (display, default_cmap, grey_color, &exact_def);
    }
  else
    {
#ifdef DEBUG
      printf ("allocating %s\n", name[8]);
#endif
      result = XParseColor (display, default_cmap, name[8], &exact_def);
    }

  if (result == 0)
    {
      if (strlen (grey_color) > 0)
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   grey_color);
	}
      else
	{
	  fprintf (stderr, "%s: color specification %s invalid\n", progname,
		   name[8]);
	}
      menu_pixel = WhitePixel (display, screen_num);
    }
  else
    {
      if (!XAllocColor (display, default_cmap, &exact_def))
	{
	  fprintf (stderr,
		   "%s: can't allocate color: all colorcells allocated and no matching cell found.\n",
		   progname);
	  menu_pixel = WhitePixel (display, screen_num);
	}
      else
	{
	  menu_pixel = exact_def.pixel;
	}
    }




  /* one last check to make sure the colors are different! */
  if (black_pixel == green_pixel)
    {
      printf
	("Background and foreground colors are the same!  Switching to use contrasting colors...\n");
      black_pixel = WhitePixel (display, screen_num);
      green_pixel = BlackPixel (display, screen_num);
    }

  border_pixel = turquoise_pixel;
  background_pixel = black_pixel;
  foreground_pixel = green_pixel;
  cursor_pixel = yellow_pixel;
  menuborder_pixel = border_pixel;
  return (1);
}
