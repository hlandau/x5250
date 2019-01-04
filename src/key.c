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
#include "debug.h"
#include <stdlib.h>



extern Display *display;
extern unsigned int ***buf5250;
extern screenfields *fields5250;
extern int subwindowcount;
extern int screencolumns;
extern int screenrows;
extern keymap *kmaplist;
extern char *appname;
extern XrmDatabase rDB;



int xkey_to_5250key (KeySym * keysym, unsigned int keystate, keymap * list);
keymap *keymap_new ();
keymap *keymap_add (keymap * list, keymap * node);
void set_keymap_values (unsigned char *buffer, int buflen, int key);


/* This function handles key presses after X11 has returned a keypress
 * event.
 */
int
keyhandler (int *fieldcount, int *row, int *column, XEvent * report,
	    KeySym * keysym, XComposeStatus * compose, unsigned char *buffer,
	    int windowrowoffset, int windowcolumnoffset)
{
  int mappedkey;
  int i;

  XLookupString ((XKeyEvent *) report, buffer, sizeof (buffer), keysym,
		 compose);

  /* first check for mapped keys */
  if ((mappedkey = xkey_to_5250key (keysym, report->xkey.state,
				    kmaplist)) != 0)
    {
      return (mappedkey);
    }

  /* now need to do the right thing with every keysym possibility,
   *as minimum:
   */
  if ((*keysym == XK_Return) || (*keysym == XK_Linefeed))
    {
      *fieldcount = 0;
      return K_ENTER;
    }
  else if (*keysym == XK_KP_Enter)
    {
      return K_FIELDEXIT;
    }
  else if (*keysym == XK_KP_Subtract)
    {
      return K_FIELDMINUS;
    }
  else if (*keysym == XK_KP_Add)
    {
      return K_FIELDPLUS;
    }
  else if ((*keysym >= XK_Shift_L) && (*keysym <= XK_Hyper_R))
    {
      return (-1);		/* do nothing because its a modifier key */
    }
  else if ((*keysym >= XK_ISO_Lock) && (*keysym <= XK_ISO_Last_Group_Lock))
    {
      return (-1);		/* do nothing because its a modifier key */
    }
  else if (*keysym == XK_F1)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F13;
	}
      else
	{
	  return K_F1;
	}
    }
  else if (*keysym == XK_F2)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F14;
	}
      else
	{
	  return K_F2;
	}
    }
  else if (*keysym == XK_F3)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F15;
	}
      else
	{
	  return K_F3;
	}
    }
  else if (*keysym == XK_F4)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F16;
	}
      else
	{
	  return K_F4;
	}
    }
  else if (*keysym == XK_F5)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F17;
	}
      else
	{
	  return K_F5;
	}
    }
  else if (*keysym == XK_F6)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F18;
	}
      else
	{
	  return K_F6;
	}
    }
  else if (*keysym == XK_F7)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F19;
	}
      else
	{
	  return K_F7;
	}
    }
  else if (*keysym == XK_F8)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F20;
	}
      else
	{
	  return K_F8;
	}
    }
  else if (*keysym == XK_F9)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F21;
	}
      else
	{
	  return K_F9;
	}
    }
  else if (*keysym == XK_F10)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F22;
	}
      else
	{
	  return K_F10;
	}
    }
  /* Sun keyboards have the F11 key mapped to SunXK_F36 */
#ifdef SunXK_F36
  else if (*keysym == XK_F11 || *keysym == SunXK_F36)
#else
  else if (*keysym == XK_F11)
#endif
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F23;
	}
      else
	{
	  return K_F11;
	}
    }
  /* Sun keyboards have the F12 key mapped to SunXK_F37 */
#ifdef SunXK_F37
  else if (*keysym == XK_F12 || *keysym == SunXK_F37)
#else
  else if (*keysym == XK_F12)
#endif
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_F24;
	}
      else
	{
	  return K_F12;
	}
    }
  else if (*keysym == XK_F13)
    {
      return K_F13;
    }
  else if (*keysym == XK_F14)
    {
      return K_F14;
    }
  else if (*keysym == XK_F15)
    {
      return K_F15;
    }
  else if (*keysym == XK_F16)
    {
      return K_F16;
    }
  else if (*keysym == XK_F17)
    {
      return K_F17;
    }
  else if (*keysym == XK_F18)
    {
      return K_F18;
    }
  else if (*keysym == XK_F19)
    {
      return K_F19;
    }
  else if (*keysym == XK_F20)
    {
      return K_F20;
    }
  else if (*keysym == XK_F21)
    {
      return K_F21;
    }
  else if (*keysym == XK_F22)
    {
      return K_F22;
    }
  else if (*keysym == XK_F23)
    {
      return K_F23;
    }
  else if (*keysym == XK_F24)
    {
      return K_F24;
    }
  else if ((*keysym >= XK_F25) && (*keysym <= XK_F35))
    {
      fprintf (stderr, "got a function higher than 24\n");
      return (-1);
    }
  else if (*keysym == XK_Print)
    {
      return K_PRINT;
    }
  else if (*keysym == XK_Sys_Req)
    {
      return K_SYSREQ;
    }
  else if (*keysym == XK_Escape)
    {
      return K_ATTENTION;
    }
  else if (*keysym == XK_End)
    {
      return K_END;
    }
  else if (*keysym == XK_Delete)
    {
      return K_DELETE;
    }
  else if (*keysym == XK_BackSpace)
    {
      return K_BACKSPACE;
    }
  else if (*keysym == XK_Tab)
    {
      return K_TAB;
    }
  else if (*keysym == XK_ISO_Left_Tab)
    {
      return K_BACKTAB;
    }
  else if (*keysym == XK_Right)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_NEXTWORD;
	}
      else
	{
	  return K_RIGHT;
	}
    }
  else if (*keysym == XK_Left)
    {
      if ((report->xkey.state & ShiftMask) != 0)
	{
	  return K_PREVWORD;
	}
      else
	{
	  return K_LEFT;
	}
    }
  else if (*keysym == XK_Up)
    {
      return K_UP;
    }
  else if (*keysym == XK_Home)
    {
      return K_HOME;
    }
  else if (*keysym == XK_Down)
    {
      return K_DOWN;
    }
  else if (*keysym == XK_Next)
    {
      return K_ROLLUP;
    }
  else if (*keysym == XK_Prior)
    {
      return K_ROLLDN;
    }
  else if (*keysym == XK_Insert)
    {
      return K_INSERT;
    }




  if ((*keysym == XK_d) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_DUPLICATE;
    }
  if ((*keysym == XK_r) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_RESET;
    }
  if ((*keysym == XK_h) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_HELP;
    }
  if ((*keysym == XK_n) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_NEWLINE;
    }
  if ((*keysym == XK_x) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_FIELDEXIT;
    }
  if ((*keysym == XK_e) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_ERASE;
    }
  if ((*keysym == XK_l) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_REFRESH;
    }
#ifdef MACRO_SUPPORT
  if ((*keysym == XK_s) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_MEMO;
    }
#endif
  if ((*keysym == XK_t) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_TESTREQ;
    }
#ifdef MACRO_SUPPORT
  if ((*keysym == XK_w) && ((report->xkey.state & ControlMask) != 0))
    {
      return K_EXEC;
    }
#endif
  if ((*keysym == XK_q) && ((report->xkey.state & ControlMask) != 0))
    {
      return (-2);
    }




  /* If the current cursor location is not in a field, get outta here
   * (this needs to be changed to indicate input prohibited)
   */
  if (!(is_in_field (*row - windowrowoffset, *column - windowcolumnoffset,
		     fieldcount)))
    {
      return (-1);
    }

  if (((*keysym >= XK_KP_Space) && (*keysym <= XK_KP_9))
      || ((*keysym >= XK_space) && (*keysym <= XK_ydiaeresis)))
    {
      if (strlen (buffer) > 1)
	{
	  fprintf (stderr, "Unknown key: %s.  Trying %c.\n", buffer, buffer[1]);
	  fprintf (stderr, "Hex codes received: ");

	  for (i = 0; i < strlen (buffer); i++)
	    {
	      fprintf (stderr, "%x", buffer[i]);
	    }

	  fprintf (stderr, "\n");
	  XBell (display, 100);
	  return (buffer[1]);
	  /*return 0;*/
	}
      else
	{
	  return (buffer[0]);
	}
    }
  else
    {
      /*
         fprintf (stderr, "keysym %s is not handled\n",
         XKeysymToString (*keysym));
         XBell (display, 100);
         return (-1);
       */
      /* We got a key that we don't understand.  Return the integer value
       * of the keysym and let lib5250 try to figure it out.
       *
       * I don't know if this actually works - just trying to get more i18n
       * working.
       */
      return (int) *keysym;
    }
}



int
xkey_to_5250key (KeySym * keysym, unsigned int keystate, keymap * list)
{
  unsigned int modifiermask;
  keymap *iter;

  /* ignore the num lock mask (Mod2Mask) and caps lock mask (LockMask) */
  modifiermask =
    ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask;
  modifiermask = modifiermask & keystate;

  if ((iter = list) != NULL)
    {
      do
	{
	  if ((iter->keysym == *keysym) && (iter->state == modifiermask))
	    {
	      return (iter->tn5250key);
	    }
	  iter = iter->next;
	}
      while (iter != list);
    }
  return (0);
}



void
load_keymap ()
{
  char *str_type[20];
  char namestring[50];
  XrmValue value;
  unsigned char *buffer;

  if (appname == NULL)
    {
      appname = malloc (6);
      sprintf (appname, "%s", "x5250");
      appname[5] = '\0';
    }

  sprintf (namestring, "%s.enterkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Enterkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_ENTER);
      free (buffer);
    }

  sprintf (namestring, "%s.attentionkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Attentionkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_ATTENTION);
      free (buffer);
    }

  sprintf (namestring, "%s.sysrequestkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Sysrequestkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_SYSREQ);
      free (buffer);
    }

  sprintf (namestring, "%s.resetkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Resetkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_RESET);
      free (buffer);
    }

  sprintf (namestring, "%s.helpkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Helpkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_HELP);
      free (buffer);
    }

  sprintf (namestring, "%s.newlinekey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Newlinekey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_NEWLINE);
      free (buffer);
    }

  sprintf (namestring, "%s.dupkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Dupkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_DUPLICATE);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldexitkey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Fieldexitkey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_FIELDEXIT);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldpluskey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Fieldpluskey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_FIELDPLUS);
      free (buffer);
    }

  sprintf (namestring, "%s.fieldminuskey", appname);
  if (XrmGetResource (rDB, namestring, "X5250.Fieldminuskey",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_FIELDMINUS);
      free (buffer);
    }

  sprintf (namestring, "%s.f1key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F1key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F1);
      free (buffer);
    }

  sprintf (namestring, "%s.f2key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F2key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F2);
      free (buffer);
    }

  sprintf (namestring, "%s.f3key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F3key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F3);
      free (buffer);
    }

  sprintf (namestring, "%s.f4key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F4key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F4);
      free (buffer);
    }

  sprintf (namestring, "%s.f5key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F5key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F5);
      free (buffer);
    }

  sprintf (namestring, "%s.f6key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F6key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F6);
      free (buffer);
    }

  sprintf (namestring, "%s.f7key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F7key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F7);
      free (buffer);
    }

  sprintf (namestring, "%s.f8key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F8key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F8);
      free (buffer);
    }

  sprintf (namestring, "%s.f9key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F9key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F9);
      free (buffer);
    }

  sprintf (namestring, "%s.f10key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F10key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F10);
      free (buffer);
    }

  sprintf (namestring, "%s.f11key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F11key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F11);
      free (buffer);
    }

  sprintf (namestring, "%s.f12key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F12key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F12);
      free (buffer);
    }

  sprintf (namestring, "%s.f13key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F13key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F13);
      free (buffer);
    }

  sprintf (namestring, "%s.f14key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F14key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F14);
      free (buffer);
    }

  sprintf (namestring, "%s.f15key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F15key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F15);
      free (buffer);
    }

  sprintf (namestring, "%s.f16key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F16key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F16);
      free (buffer);
    }

  sprintf (namestring, "%s.f17key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F17key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F17);
      free (buffer);
    }

  sprintf (namestring, "%s.f18key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F18key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F18);
      free (buffer);
    }

  sprintf (namestring, "%s.f19key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F19key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F19);
      free (buffer);
    }

  sprintf (namestring, "%s.f20key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F20key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F20);
      free (buffer);
    }

  sprintf (namestring, "%s.f21key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F21key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F21);
      free (buffer);
    }

  sprintf (namestring, "%s.f22key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F22key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F22);
      free (buffer);
    }

  sprintf (namestring, "%s.f23key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F23key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F23);
      free (buffer);
    }

  sprintf (namestring, "%s.f24key", appname);
  if (XrmGetResource (rDB, namestring, "X5250.F24key",
		      str_type, &value) == True)
    {
      buffer = malloc (value.size + 1);
      strncpy (buffer, value.addr, (int) value.size);
      buffer[value.size] = '\0';
      set_keymap_values (buffer, value.size, K_F24);
      free (buffer);
    }
  return;
}



keymap *
keymap_new ()
{
  keymap *This = tn5250_new (keymap, 1);
  if (This == NULL)
    {
      return NULL;
    }
  memset (This, 0, sizeof (keymap));
  This->next = NULL;
  This->prev = NULL;
  return (This);
}



keymap *
keymap_add (keymap * list, keymap * node)
{
  node->prev = node->next = NULL;

  if (list == NULL)
    {
      node->next = node->prev = node;
      return node;
    }
  node->next = list;
  node->prev = list->prev;
  node->prev->next = node;
  node->next->prev = node;
  return list;
}



void
set_keymap_values (unsigned char *buffer, int buflen, int key)
{
  char *token;
  char *nomodtoken, *modtoken;
  char *keystring = malloc (buflen + 1);
  char space[2];
  char delim[2];

  if (buffer == NULL)
    {
      return;
    }

  delim[0] = '(';
  delim[1] = '\0';
  memcpy (keystring, buffer, buflen);
  nomodtoken = strsep (&keystring, delim);

  /* Get any non-combination keymaps first (maps without modifier keys) */
  if (strlen (nomodtoken) > 0)
    {
      space[0] = ' ';
      space[1] = '\0';
      token = strtok (nomodtoken, space);

      if (token != NULL)
	{
	  keymap *kmap = keymap_new ();
	  kmap->keysym = strtol (token, (char **) NULL, 16);
	  kmap->state = 0;
	  kmap->tn5250key = key;
	  kmaplist = keymap_add (kmaplist, kmap);

	  space[0] = ' ';
	  space[1] = '\0';
	  while ((token = strtok (NULL, space)) != NULL)
	    {
	      keymap *kmap = keymap_new ();
	      kmap->keysym = strtol (token, (char **) NULL, 16);
	      kmap->state = 0;
	      kmap->tn5250key = key;
	      kmaplist = keymap_add (kmaplist, kmap);
	      space[0] = ' ';
	      space[1] = '\0';
	    }
	}
    }

  /* Now get any combination key maps (maps with modifiers) */
  if (keystring != NULL)
    {
      delim[0] = ')';
      delim[1] = '\0';
      modtoken = strsep (&keystring, delim);

      if (strlen (modtoken) > 0)
	{
	  space[0] = ' ';
	  space[1] = '\0';
	  token = strtok (modtoken, space);

	  if (token != NULL)
	    {
	      keymap *kmap = keymap_new ();
	      kmap->keysym = strtol (token, (char **) NULL, 16);
	      kmap->state = 0;
	      kmap->tn5250key = key;

	      space[0] = ' ';
	      space[1] = '\0';
	      while ((token = strtok (NULL, space)) != NULL)
		{
		  kmap->state = kmap->state | kmap->keysym;
		  kmap->keysym = strtol (token, (char **) NULL, 16);
		  kmap->tn5250key = key;
		  space[0] = ' ';
		  space[1] = '\0';
		}
	      kmaplist = keymap_add (kmaplist, kmap);
	    }
	}
    }

  if ((keystring != NULL) && (strlen (keystring) > 0))
    {
      set_keymap_values (keystring, strlen (keystring), key);
    }
  return;
}
