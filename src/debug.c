#include "config.h"
#include "x5250.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>


extern unsigned int ***buf5250;
extern int screencolumns;
extern int screenrows;


int
fillbuf (int instance, int subwindowcount)
{
  char buffer[133];
  int i;

  screencolumns = 80;
  screenrows = 24;
  subwindowcount = 0;
  destroy_screen (subwindowcount);

  sprintf (buffer, "Welcome to x5250.");

  for (i = 0; i < strlen (buffer); i++)
    {
      buf5250[subwindowcount][2][((screencolumns / 2) - ((strlen (buffer)) / 2)) + i] = buffer[i] | A_X5250_WHITE | A_X5250_BOLD;
    }

  sprintf (buffer, "Use the right mouse button anywhere on");

  for (i = 0; i < strlen (buffer); i++)
    {
      buf5250[subwindowcount][5][((screencolumns / 2) - ((strlen (buffer)) / 2)) + i] = buffer[i] | A_X5250_GREEN | A_X5250_NORMAL;
    }

  sprintf (buffer, "this window to access the menu.");

  for (i = 0; i < strlen (buffer); i++)
    {
      buf5250[subwindowcount][6][((screencolumns / 2) - ((strlen (buffer)) / 2)) + i] = buffer[i] | A_X5250_GREEN | A_X5250_NORMAL;
    }

  sprintf (buffer, "Copyright (C) 2004 James Rich");

  for (i = 0; i < strlen (buffer); i++)
    {
      buf5250[subwindowcount][7][((screencolumns / 2) - ((strlen (buffer)) / 2)) + i] = buffer[i] | A_X5250_GREEN | A_X5250_NORMAL;
    }

  return (subwindowcount);
}



void
dump_buffer (int bufferid)
{
  int i, j;

  for (i = 0; i < 27; i++)
    {
      printf ("row %d: ", i);
      for (j = 0; j < 133; j++)
	{
	  printf ("%d", buf5250[bufferid][i][j]);
	}
      printf ("\n");
    }
  return;
}




/* Gets the user defaults from the X resource database.
 */
void
dump_resource_database (XrmDatabase resourceDB, char *appname)
{
  char *str_type[20];
  char namestring[50];
  XrmValue value;
  char *buffer;

  if (appname == NULL)
    {
      appname = malloc (6);
      sprintf (appname, "%s", "x5250");
      appname[5] = '\0';
    }

  /* get the title */
  sprintf (namestring, "%s.title", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Title",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("title: %s\n", buffer);
      free (buffer);
    }

  /* get geometry (actually, this is currently ignored) */
  sprintf (namestring, "%s.geometry", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Geometry",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("geometry: %s\n", buffer);
      free (buffer);
    }

  /* get the font */
  sprintf (namestring, "%s.font", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Font",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("font: %s\n", buffer);
      free (buffer);
    }

  /* get the 132 column font */
  sprintf (namestring, "%s.font132", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Font132",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("font132: %s\n", buffer);
      free (buffer);
    }

  /* get the bell volume */
  sprintf (namestring, "%s.bell", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Bell",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("bell: %s\n", buffer);
      free (buffer);
    }

  /* get the paste method */
  sprintf (namestring, "%s.pastecursor", appname);
  if (XrmGetResource(resourceDB, namestring, "X5250.Pastecursor",
		     str_type, &value) == True)
    {
      if (strncmp(value.addr, "True", (int) value.size) == 0)
	{
	  printf ("pastecursor: True\n");
	}
      else
	{
	  printf ("pastecursor: False\n");
	}
    }

  /* check for a ruler */
  sprintf (namestring, "%s.ruler", appname);
  if (XrmGetResource(resourceDB, namestring, "X5250.Ruler",
		     str_type, &value) == True)
    {
      if (strncmp(value.addr, "True", (int) value.size) == 0)
	{
	  printf ("ruler: True\n");
	}
      else
	{
	  printf ("ruler: False\n");
	}
    }

  /* check for column separators */
  sprintf (namestring, "%s.columnseparators", appname);
  if (XrmGetResource(resourceDB, namestring, "X5250.Columnseparators",
		     str_type, &value) == True)
    {
      if (strncmp(value.addr, "True", (int) value.size) == 0)
	{
	  printf ("columnseparators: True\n");
	}
      else
	{
	  printf ("columnseparators: False\n");
	}
    }

  /* get the column separator style */
  sprintf (namestring, "%s.separatordots", appname);
  if (XrmGetResource(resourceDB, namestring, "X5250.Separatordots",
		     str_type, &value) == True)
    {
      if (strncmp(value.addr, "True", (int) value.size) == 0)
	{
	  printf ("separatordots: True\n");
	}
      else
	{
	  printf ("separatordots: False\n");
	}
    }

  /* Use the foreground color as the color for green */
  sprintf (namestring, "%s.foreground", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Foreground",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("foreground: %s\n", buffer);
      free (buffer);
    }

  /* Use the black color as the background */
  sprintf (namestring, "%s.background", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Background",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("background: %s\n", buffer);
      free (buffer);
    }

  /* These colors override the foreground and background settings above,
   * so we do these second.  This might fix problems some platforms have
   * that set a global *foreground and *background in the X environment.
   */
  sprintf (namestring, "%s.redcolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Redcolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("red color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.greencolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Greencolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("green color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.bluecolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Bluecolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("blue color: %s\n", buffer);
      free (buffer);
    }

  /* Decided to get rid of this resource as it is probably too confusing
   * to have both blackcolor and background.  Just use background since
   * there is no color for black in the 5250 protocol.
   */
  /*
  sprintf (namestring, "%s.blackcolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Blackcolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("black color: %s\n", buffer);
      free (buffer);
    }
  */

  sprintf (namestring, "%s.whitecolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Whitecolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("white color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.yellowcolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Yellowcolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("yellow color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.turquoisecolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Turquoisecolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("turquoise color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.pinkcolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Pinkcolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("pink color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.greycolor", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Greycolor",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("grey color: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.enterkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Enterkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("enter key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.attentionkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Attentionkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("attention key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.sysrequestkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Sysrequestkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("system request key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.resetkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Resetkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("reset key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.helpkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Helpkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("help key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.newlinekey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Newlinekey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("new line key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.dupkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Dupkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("dup key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldexitkey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Fieldexitkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("field exit key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldpluskey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Fieldpluskey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("field plus key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldminuskey", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.Fieldminuskey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("field minus key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f1key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F1key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F1 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f2key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F2key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F1 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f3key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F3key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F1 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f4key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F4key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F4 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f5key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F5key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F5 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f6key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F6key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F6 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f7key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F7key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F7 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f8key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F8key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F8 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f9key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F9key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F9 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f10key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F10key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F10 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f11key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F11key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F11 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f12key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F12key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F12 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f13key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F13key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F13 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f14key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F14key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F14 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f15key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F15key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F15 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f16key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F16key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F16 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f17key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F17key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F17 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f18key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F18key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F18 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f19key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F19key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F19 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f20key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F20key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F20 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f21key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F21key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F21 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f22key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F22key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F22 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f23key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F23key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F23 key: %s\n", buffer);
      free (buffer);
    }

  sprintf (namestring, "%s.f24key", appname);
  if (XrmGetResource (resourceDB, namestring, "X5250.F24key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      printf ("F24 key: %s\n", buffer);
      free (buffer);
    }

  return;
}




int
is_font_scalable (char *name)
{
  int i, field, length;

  if ((name == NULL ) || (name[0] != '-'))
    {
      /*
      printf ("Not a valid scalable font\n");
      */
      return (0);
    }

  length = strlen (name);

  for (i = field = 0; (name[i] != '\0') && (i < length); i++)
    {
      if (name[i] == '-')
	{
	  field++;

	  if ((field == 7) || (field == 8) || (field == 12))
	    {
	      if ((name[i + 1] != '0') || (name[i + 2] != '-'))
		{
		  /*
		  printf ("Not a valid scalable font\n");
		  */
		  return (0);
		}
	    }
	}
    }

  if (field != 14)
    {
      /*
      printf ("Not a valid scalable font\n");
      */
      return (0);
    }

  return (1);
}




int
font_point_size (char *name)
{
  int i, j, field, length;
  char pointsize[50] = {'\0'};

  if ((name == NULL ) || (name[0] != '-'))
    {
      return (0);
    }

  length = strlen (name);

  for (i = field = 0; (name[i] != '\0') && (i < length); i++)
    {
      if (name[i] == '-')
	{
	  field++;

	  if ((field == 7) || (field == 8) || (field == 12))
	    {
	      if ((name[i + 1] != '0') || (name[i + 2] != '-'))
		{
		  return (0);
		}

	      if (field == 8)
		{
		  for (j = 0; ((name[i + 1] != '-') && (i + 1 < length)); j++)
		    {
		      pointsize[j] = name[i + 1];
		      i++;
		    }
		  pointsize[j] = '\0';
		}
	    }
	}
    }

  if (field != 14)
    {
      return (0);
    }

  if (strlen (pointsize) > 0)
    {
      return (atoi (pointsize));
    }

  return (0);
}
