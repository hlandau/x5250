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
#include "help.h"
#include <stdio.h>


extern char *progname;


void
usage ()
{
  printf ("x5250 - TCP/IP 5250 emulator\n");
  printf ("Syntax:\n");
  printf ("%s [OPTION] HOST[:PORT]\n", progname);
#ifdef HAVE_LIBSSL
  printf ("To connect using ssl prefix HOST with 'ssl:'.  Example:\n");
  printf ("\t%s +ssl_verify_server ssl:as400.example.com\n", progname);
#endif
  printf ("\nOptions:\n");
  printf ("  map=NAME\t\tCharacter map (default is '37'):");
  printf ("\n");
  printf ("  env.DEVNAME=NAME\tUse NAME as session name (default: none).\n");
#ifndef NDEBUG
  printf ("  trace=FILE\t\tLog session to FILE.\n");
#endif
#ifdef HAVE_LIBSSL
  printf
    ("  +/-ssl_verify_server\tVerify/don't verify the server's SSL certificate\n");
  printf
    ("  ssl_ca_file=FILE\tUse certificate authority (CA) certs from FILE\n");
  printf
    ("  ssl_cert_file=FILE\tFile containing SSL certificate in PEM format to\n");
  printf ("\t\t\tuse if the AS/400 requires client authentication.\n");
  printf
    ("  ssl_pem_pass=PHRASE\tPassphrase to use when decrypting a PEM private\n");
  printf ("\t\t\tkey.  Used in conjunction with ssl_cert_file\n");
  printf
    ("  ssl_check_exp[=SECS]\tCheck if SSL certificate is expired, or if it\n");
  printf ("\t\t\twill be expired in SECS seconds.\n");
#endif
  /*
     printf ("  +/-ruler\t\tDraw a ruler pointing to the cursor position\n");
     printf ("  +/-version\t\tShow emulator version and exit.\n");
   */
  printf ("  env.NAME=VALUE\tSet telnet environment string NAME to VALUE.\n");
  printf ("  env.TERM=TYPE\t\tEmulate IBM terminal type (default: depends)");
  printf ("\n\n");
  printf ("Use the --help option for detailed usage instructions.\n");
  return;
}




void
print_help ()
{
  printf ("Usage: x5250 [OPTION] HOST[:PORT]\n");
  printf ("5250 emulator for the X Window System.\n");
  printf ("\n");
#ifdef HAVE_GETOPT_H
  printf ("Mandatory arguments to long options are mandatory ");
  printf ("for short options too.\n");
#endif
  printf ("  -display HOST:DISPLAY\t\tdisplay to use\n");
  printf ("  -name NAME\t\t\tapplication name (default: x5250)\n");
  printf ("  -title TITLE\t\t\tapplication title\n");
/*
  printf ("  -h, --host=HOST\tconnect to remote host HOST\n");
*/
  printf ("  -geometry HxW+X+Y\t\tx5250 main window size and position in pixels.\n");
  printf ("\t\t\t\tHeight and Width and position X and Y; see the\n");
  printf ("\t\t\t\tX(7) man page.\n");
  printf ("  -bg, -background\t\tbackground color (default: black)\n");
  printf ("  -fg, -foreground\t\tforeground color (default: green)\n");
/*
  printf
    ("  -bd, -bordercolor\t\tcolor to use for window borders (default: turquoise)\n");
  printf ("  -bw, -borderwidth\t\tsize in pixels of window borders\n");
*/
  printf ("  -fn, -font\t\t\tfont (default: 7x13)\n");
  printf ("  -fn132, -font132\t\tfont for 132 columns (default: font)\n");
  printf ("  -pc, -pastecursor\t\tpaste text at cursor location instead of mouse\n\t\t\t\tpointer location\n");
  printf ("  -ruler\t\t\tdisplay a ruler with the cursor\n");
  printf ("  -cs, -columnseparators\tshow column separators\n");
  printf ("  -sd, -separatordots\t\tuse dots for column separators\n");
#ifdef HAVE_GETOPT_H
  printf ("  -R, --debug-resources\t\tprint resource debugging information to stdout\n");
  printf ("  -v, --version\t\t\tprint version information and exit\n");
  printf ("  -H, --help\t\t\tprint this help and exit\n");
#else
  printf ("  -R\t\t\tprint resource debugging information to stdout\n");
  printf ("  -v\t\t\t\tprint version information and exit\n");
  printf ("  -H\t\t\t\tprint this help and exit\n");
#endif
  printf ("\n");
  printf ("Report bugs to <james@eaerich.com>.\n");
  return;
/*
  {"-xrm", NULL, XrmoptionResArg, (XPointer) NULL},
*/
}
