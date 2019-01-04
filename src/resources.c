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
#include "x5250term.h"
#include "resources.h"
#include "help.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>


#ifndef DATADIR
#define DATADIR "/usr/X11/lib/X11/app-defaults"
#endif


extern Display *display;
extern char *display_name;
extern int screen_num;
extern Screen *screen_ptr;
extern unsigned long foreground_pixel, background_pixel, border_pixel;
extern unsigned long menu_pixel, menuborder_pixel;
extern unsigned long red_pixel, green_pixel, blue_pixel, black_pixel,
  white_pixel, yellow_pixel, turquoise_pixel, pink_pixel;
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
extern char *sessionname;
extern int placementx;
extern int placementy;
extern int mainwindowwidth;
extern int mainwindowheight;
extern char *font;
extern char *font132;
extern char *fontui;
extern Bool separatordots;
extern Bool debugresources;
extern int globalargc;
extern char **globalargv;
extern XrmDatabase rDB;


/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XrmParseCommand is let loose. 
   We don't do anything with many of these resources, but the program is ready
   for expansion */

#define GEOMETRY	"*geometry"

static int opTableEntries = 26;
static XrmOptionDescRec opTable[] = {
  {"-geometry", GEOMETRY, XrmoptionSepArg, (XPointer) NULL},
  {"-background", "*background", XrmoptionSepArg, (XPointer) NULL},
  {"-bg", "*background", XrmoptionSepArg, (XPointer) NULL},
  {"-fg", "*foreground", XrmoptionSepArg, (XPointer) NULL},
  {"-foreground", "*foreground", XrmoptionSepArg, (XPointer) NULL},
  {"-xrm", NULL, XrmoptionResArg, (XPointer) NULL},
  {"-display", ".display", XrmoptionSepArg, (XPointer) NULL},
  {"-bd", "*borderColor", XrmoptionSepArg, (XPointer) NULL},
  {"-bordercolor", "*borderColor", XrmoptionSepArg, (XPointer) NULL},
  {"-borderwidth", ".borderWidth", XrmoptionSepArg, (XPointer) NULL},
  {"-bw", ".borderWidth", XrmoptionSepArg, (XPointer) NULL},
  {"-fn", "*font", XrmoptionSepArg, (XPointer) NULL},
  {"-font", "*font", XrmoptionSepArg, (XPointer) NULL},
  {"-fn132", "*font132", XrmoptionSepArg, (XPointer) NULL},
  {"-font132", "*font132", XrmoptionSepArg, (XPointer) NULL},
  {"-fnui", "*fontui", XrmoptionSepArg, (XPointer) NULL},
  {"-fontui", "*fontui", XrmoptionSepArg, (XPointer) NULL},
  {"-name", ".name", XrmoptionSepArg, (XPointer) NULL},
  {"-title", ".title", XrmoptionSepArg, (XPointer) NULL},
  {"-pc", "*pastecursor", XrmoptionNoArg, (XPointer) "True"},
  {"-pastecursor", "*pastecursor", XrmoptionNoArg, (XPointer) "True"},
  {"-ruler", "*ruler", XrmoptionNoArg, (XPointer) "True"},
  {"-cs", "*columnseparators", XrmoptionNoArg, (XPointer) "True"},
  {"-columnseparators", "*columnseparators", XrmoptionNoArg,
   (XPointer) "True"},
  {"-sd", "*separatordots", XrmoptionNoArg, (XPointer) "True"},
  {"-separatordots", "*separatordots", XrmoptionNoArg, (XPointer) "True"},
};

static XrmDatabase commandlineDB;
extern char *appname;



/* Gets the user's home directory.
 */
static char *gethomedir (char *dest);




void
parse_command_line_X ()
{
  XrmValue value;
  char *str_type[20];
#ifdef DEBUG
  int i;
#endif

  /*
     XrmParseCommand (&commandlineDB, opTable, opTableEntries,
     progname, &globalargc, globalargv);
   */
  XrmParseCommand (&commandlineDB, opTable, opTableEntries,
		   "x5250", &globalargc, globalargv);

  /* get display now, because we need it to get other databases */
  if (XrmGetResource (commandlineDB, "x5250.display",
		      "X5250.Display", str_type, &value) == True)
    {
      display_name = malloc (value.size + 1);
      strncpy (display_name, value.addr, (int) value.size);
      display_name[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("display name: %s\n", display_name);
	}
    }

  /* Get the application name preference */
  if (XrmGetResource (commandlineDB, "x5250.name", "X5250.Name",
		      str_type, &value) == True)
    {
      appname = malloc (value.size + 1);
      strncpy (appname, value.addr, (int) value.size);
      appname[value.size] = '\0';
#ifdef DEBUG
      printf ("name: %s\n", appname);
#endif
    }

#ifdef DEBUG
  printf ("command line arguments (%d total):", globalargc);
  for (i = 0; i < globalargc; i++)
    {
      printf (" %s", globalargv[i]);
    }
  printf ("\n");
#endif
  return;
}




/* Merges the X resource databases.
 */
void
merge_db_X ()
{
  XrmDatabase homeDB, serverDB, applicationDB;

  char filenamebuf[1024];
  char *filename = &filenamebuf[0];
  char *environment;

  /* get application defaults file, if any */
  applicationDB = XrmGetFileDatabase (DATADIR "/x5250/X5250");
  if (debugresources == True)
    {
      printf ("Application resource database:\n");
      dump_resource_database (applicationDB, appname);
    }
  XrmMergeDatabases (applicationDB, &rDB);

  /* MERGE server defaults, these are created by xrdb, loaded as a
   * property of the root window when the server initializes, and
   * loaded into the display structure on XOpenDisplay.  If not defined,
   * use .Xdefaults  */
  if (XResourceManagerString (display) != NULL)
    {
      serverDB = XrmGetStringDatabase (XResourceManagerString (display));
    }
  else
    {
      /* Open .Xdefaults file and merge into existing data base */
      gethomedir (filename);
      strcat (filename, "/.Xdefaults");

      serverDB = XrmGetFileDatabase (filename);
    }
  if (debugresources == True)
    {
      printf ("Server resource database:\n");
      dump_resource_database (serverDB, appname);
    }
  XrmMergeDatabases (serverDB, &rDB);

  /* Open XENVIRONMENT file, or if not defined, the ~/.Xdefaults,
   * and merge into existing data base */
  if ((environment = getenv ("XENVIRONMENT")) == NULL)
    {
      int len;
      environment = gethomedir (filename);
      strcat (environment, "/.Xdefaults-");
      len = strlen (environment);
      gethostname (environment + len, 1024 - len);
    }
  homeDB = XrmGetFileDatabase (environment);
  if (debugresources == True)
    {
      printf ("User resource database:\n");
      dump_resource_database (homeDB, appname);
    }
  XrmMergeDatabases (homeDB, &rDB);

  /* command line takes precedence over everything */
  if (debugresources == True)
    {
      printf ("Command line resource database:\n");
      dump_resource_database (commandlineDB, "x5250");
      printf ("\n");
    }
  XrmMergeDatabases (commandlineDB, &rDB);
  return;
}




/* Gets the user defaults from the X resource database.
 */
void
get_resources_X (Tn5250Terminal * terminal)
{
  char *str_type[20];
  char namestring[50];
  long flags;
  XrmValue value;
  char *geometry = NULL;
  char *buffer;
  char key[100];

  if (debugresources == True)
    {
      printf ("Resources after merging databases:\n");
    }

  if (appname == NULL)
    {
      appname = malloc (6);
      sprintf (appname, "%s", "x5250");
      appname[5] = '\0';
    }

  /* get the title */
  sprintf (namestring, "%s.title", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Title",
		      str_type, &value) == True)
    {
      sessionname = malloc (value.size + 1);
      strncpy (sessionname, value.addr, (int) value.size);
      sessionname[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("title: %s\n", sessionname);
	}
    }

  /* get geometry */
  sprintf (namestring, "%s.geometry", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Geometry",
		      str_type, &value) == True)
    {
      geometry = malloc (value.size + 1);
      strncpy (geometry, value.addr, (int) value.size);
      geometry[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("geometry: %s\n", geometry);
	}

      flags =
	XParseGeometry (geometry, &placementx, &placementy, &mainwindowwidth,
			&mainwindowheight);

      /*
         if ((WidthValue | HeightValue) & flags)
         {
         printf ("geometry: %dx%d+%d+%d\n", mainwindowwidth, mainwindowheight,
         placementx, placementy);
         }
       */

      /* Position values are for bottom right if negative, so calculate where
       * placement should be.
       */
      if (XValue & flags)
	{
	  if (XNegative & flags)
	    {
	      placementx =
		DisplayWidth (display,
			      screen_num) + placementx - mainwindowwidth;
	    }
	}
      if (YValue & flags)
	{
	  if (YNegative & flags)
	    {
	      placementy =
		DisplayHeight (display,
			       screen_num) + placementy - mainwindowheight;
	    }
	  terminal->data->size_hints->flags |= USPosition;
	  terminal->data->size_hints->y = placementy;
	}
    }
  else
    {
      mainwindowwidth = 0;
      mainwindowheight = 0;
      placementx = 0;
      placementy = 0;
    }

  /* get the font */
  sprintf (namestring, "%s.font", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Font",
		      str_type, &value) == True)
    {
      font = malloc (value.size + 1);
      strncpy (font, value.addr, (int) value.size);
      font[value.size] = '\0';
    }
  else
    {
      font = malloc (strlen ("7x13") + 1);
      sprintf (font, "%s", "7x13");
    }
  if (debugresources == True)
    {
      printf ("font: %s\n", font);
    }

  /* get the 132 column font */
  sprintf (namestring, "%s.font132", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Font132",
		      str_type, &value) == True)
    {
      font132 = malloc (value.size + 1);
      strncpy (font132, value.addr, (int) value.size);
      font132[value.size] = '\0';
    }
  else
    {
      font132 = malloc (strlen (font) + 1);
      sprintf (font132, "%s", font);
    }
  if (debugresources == True)
    {
      printf ("font132: %s\n", font132);
    }

  /* get the menu font */
  sprintf (namestring, "%s.fontui", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Fontui",
		      str_type, &value) == True)
    {
      fontui = malloc (value.size + 1);
      strncpy (fontui, value.addr, (int) value.size);
      fontui[value.size] = '\0';
    }
  else
    {
      fontui = malloc (strlen (font) + 1);
      sprintf (fontui, "%s", font);
    }
  if (debugresources == True)
    {
      printf ("fontui: %s\n", fontui);
    }

  /* get the bell volume */
  sprintf (namestring, "%s.bell", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Bell",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      terminal->data->bellvolume = atoi (buffer);
      if (terminal->data->bellvolume > 100)
	{
	  terminal->data->bellvolume = 100;
	}
      if (terminal->data->bellvolume < -100)
	{
	  terminal->data->bellvolume = -100;
	}
      free (buffer);
      if (debugresources == True)
	{
	  printf ("bell: %d\n", terminal->data->bellvolume);
	}
    }
  else
    {
      terminal->data->bellvolume = 100;
    }

  /* get the paste method */
  sprintf (namestring, "%s.pastecursor", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Pastecursor",
		      str_type, &value) == True)
    {
      if (strncmp (value.addr, "True", (int) value.size) == 0)
	{
	  terminal->data->pastecursor = True;
	  if (debugresources == True)
	    {
	      printf ("pastecursor: True\n");
	    }
	}
      else
	{
	  terminal->data->pastecursor = False;
	  if (debugresources == True)
	    {
	      printf ("pastecursor: False\n");
	    }
	}
    }
  else
    {
      terminal->data->pastecursor = False;
      if (debugresources == True)
	{
	  printf ("pastecursor: False\n");
	}
    }

  /* check for a ruler */
  sprintf (namestring, "%s.ruler", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Ruler",
		      str_type, &value) == True)
    {
      if (strncmp (value.addr, "True", (int) value.size) == 0)
	{
	  terminal->data->ruler = True;
	  if (debugresources == True)
	    {
	      printf ("ruler: True\n");
	    }
	}
      else
	{
	  terminal->data->ruler = False;
	  if (debugresources == True)
	    {
	      printf ("ruler: False\n");
	    }
	}
    }
  else
    {
      terminal->data->ruler = False;
      if (debugresources == True)
	{
	  printf ("ruler: False\n");
	}
    }

  /* check for column separators */
  sprintf (namestring, "%s.columnseparators", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Columnseparators",
		      str_type, &value) == True)
    {
      if (strncmp (value.addr, "True", (int) value.size) == 0)
	{
	  terminal->data->columnseparators = True;
	  if (debugresources == True)
	    {
	      printf ("columnseparators: True\n");
	    }
	}
      else
	{
	  terminal->data->columnseparators = False;
	  if (debugresources == True)
	    {
	      printf ("columnseparators: False\n");
	    }
	}
    }
  else
    {
      terminal->data->columnseparators = False;
      if (debugresources == True)
	{
	  printf ("columnseparators: False\n");
	}
    }

  /* get the column separator style */
  sprintf (namestring, "%s.separatordots", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Separatordots",
		      str_type, &value) == True)
    {
      if (strncmp (value.addr, "True", (int) value.size) == 0)
	{
	  separatordots = True;
	  if (debugresources == True)
	    {
	      printf ("separatordots: True\n");
	    }
	}
      else
	{
	  separatordots = False;
	  if (debugresources == True)
	    {
	      printf ("separatordots: False\n");
	    }
	}
    }
  else
    {
      separatordots = False;
      if (debugresources == True)
	{
	  printf ("separatordots: False\n");
	}
    }

  /* Use the foreground color as the color for green */
  sprintf (namestring, "%s.foreground", appname);
  memset (green_color, '\0', sizeof (green_color));
  if (XrmGetResource (rDB, namestring, "X5250.Foreground",
		      str_type, &value) == True)
    {
      strncpy (green_color, value.addr, (int) value.size);
      green_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("foreground: %s\n", green_color);
	}

    }

  /* Use the black color as the background */
  sprintf (namestring, "%s.background", appname);
  memset (black_color, '\0', sizeof (black_color));
  if (XrmGetResource (rDB, namestring, "X5250.Background",
		      str_type, &value) == True)
    {
      strncpy (black_color, value.addr, (int) value.size);
      black_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("background: %s\n", black_color);
	}
    }

  /* These colors override the foreground and background settings above,
   * so we do these second.  This might fix problems some platforms have
   * that set a global *foreground and *background in the X environment.
   */
  sprintf (namestring, "%s.redcolor", appname);
  memset (red_color, '\0', sizeof (red_color));
  if (XrmGetResource (rDB, namestring, "X5250.Redcolor",
		      str_type, &value) == True)
    {
      strncpy (red_color, value.addr, (int) value.size);
      red_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("red color: %s\n", red_color);
	}
    }

  sprintf (namestring, "%s.greencolor", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Greencolor",
		      str_type, &value) == True)
    {
      memset (green_color, '\0', sizeof (green_color));
      strncpy (green_color, value.addr, (int) value.size);
      green_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("green color: %s\n", green_color);
	}
    }

  sprintf (namestring, "%s.bluecolor", appname);
  memset (blue_color, '\0', sizeof (blue_color));
  if (XrmGetResource (rDB, namestring, "X5250.Bluecolor",
		      str_type, &value) == True)
    {
      strncpy (blue_color, value.addr, (int) value.size);
      blue_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("blue color: %s\n", blue_color);
	}
    }

  /* Decided to get rid of this resource as it is probably too confusing
   * to have both blackcolor and background.  Just use background since
   * there is no color for black in the 5250 protocol.
   */
  /*
     sprintf (namestring, "%s.blackcolor", appname);
     if (XrmGetResource (rDB, namestring, "X5250.Blackcolor",
     str_type, &value) == True)
     {
     memset (black_color, '\0', sizeof (black_color));
     strncpy (black_color, value.addr, (int) value.size);
     black_color[value.size] = '\0';
     if (debugresources == True)
     {
     printf ("black color: %s\n", black_color);
     }
     }
   */

  sprintf (namestring, "%s.whitecolor", appname);
  memset (white_color, '\0', sizeof (white_color));
  if (XrmGetResource (rDB, namestring, "X5250.Whitecolor",
		      str_type, &value) == True)
    {
      strncpy (white_color, value.addr, (int) value.size);
      white_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("white color: %s\n", white_color);
	}
    }

  sprintf (namestring, "%s.yellowcolor", appname);
  memset (yellow_color, '\0', sizeof (yellow_color));
  if (XrmGetResource (rDB, namestring, "X5250.Yellowcolor",
		      str_type, &value) == True)
    {
      strncpy (yellow_color, value.addr, (int) value.size);
      yellow_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("yellow color: %s\n", yellow_color);
	}
    }

  sprintf (namestring, "%s.turquoisecolor", appname);
  memset (turquoise_color, '\0', sizeof (turquoise_color));
  if (XrmGetResource (rDB, namestring, "X5250.Turquoisecolor",
		      str_type, &value) == True)
    {
      strncpy (turquoise_color, value.addr, (int) value.size);
      turquoise_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("turquoise color: %s\n", turquoise_color);
	}
    }

  sprintf (namestring, "%s.pinkcolor", appname);
  memset (pink_color, '\0', sizeof (pink_color));
  if (XrmGetResource (rDB, namestring, "X5250.Pinkcolor",
		      str_type, &value) == True)
    {
      strncpy (pink_color, value.addr, (int) value.size);
      pink_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("pink color: %s\n", pink_color);
	}
    }

  sprintf (namestring, "%s.greycolor", appname);
  memset (grey_color, '\0', sizeof (grey_color));
  if (XrmGetResource (rDB, namestring, "X5250.Greycolor",
		      str_type, &value) == True)
    {
      strncpy (grey_color, value.addr, (int) value.size);
      grey_color[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("grey color: %s\n", grey_color);
	}
    }



  /* Get the key mapping resources */

  sprintf (namestring, "%s.enterkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Enterkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("enter key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.attentionkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Attentionkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("attention key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.sysrequestkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Sysrequestkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("system request key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.resetkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Resetkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("reset key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.helpkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Helpkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("help key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.newlinekey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Newlinekey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("new line key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.dupkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Dupkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("dup key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.fieldexitkey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Fieldexitkey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("field exit key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.fieldpluskey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Fieldpluskey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("field plus key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.fieldminuskey", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.Fieldminuskey",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("field minus key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f1key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F1key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F1 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f2key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F2key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F2 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f3key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F3key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F3 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f4key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F4key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F4 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f5key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F5key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F5 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f6key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F6key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F6 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f7key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F7key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F7 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f8key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F8key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F8 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f9key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F9key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F9 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f10key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F10key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F10 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f11key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F11key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F11 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f12key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F12key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F12 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f13key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F13key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F13 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f14key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F14key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F14 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f15key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F15key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F15 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f16key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F16key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F16 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f17key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F17key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F17 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f18key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F18key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F18 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f19key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F19key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F19 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f20key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F20key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F20 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f21key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F21key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F21 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f22key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F22key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F22 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f23key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F23key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F23 key: %s\n", key);
	}
    }

  sprintf (namestring, "%s.f24key", appname);
  memset (key, '\0', sizeof (key));
  if (XrmGetResource (rDB, namestring, "X5250.F24key",
		      str_type, &value) == True)
    {
      strncpy (key, value.addr, (int) value.size);
      key[value.size] = '\0';
      if (debugresources == True)
	{
	  printf ("F24 key: %s\n", key);
	}
    }
  return;
}





/* Gets the user's home directory.
 */
static char *
gethomedir (char *dest)
{
  int uid;
  struct passwd *pw;
  char *ptr;

  if ((ptr = getenv ("HOME")) != NULL)
    {
      strcpy (dest, ptr);
    }
  else
    {
      if ((ptr = getenv ("USER")) != NULL)
	{
	  pw = getpwnam (ptr);
	}
      else
	{
	  uid = getuid ();
	  pw = getpwuid (uid);
	}
      if (pw)
	{
	  strcpy (dest, pw->pw_dir);
	}
      else
	{
	  *dest = '\0';
	}
    }
  return dest;
}
