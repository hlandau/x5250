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

/* Create a dialog window (but don't map it) */
void createdialog (Tn5250Terminal * terminal);

/* Pop up the dialog window at the current mouse position */
void popdialog (Tn5250Terminal * terminal, XEvent * report);

/* Draw the contents of the dialog window */
void drawdialog (Tn5250Terminal * terminal);

/* Create a debug dialog window (but don't map it) */
void createdebugdialog (Tn5250Terminal * terminal);

/* Pop up the debug dialog window at the current mouse position */
void popdebugdialog (Tn5250Terminal * terminal, XEvent * report);

/* Draw the contents of the debug dialog window */
void drawdebugdialog (Tn5250Terminal * terminal);

/* Create a macro dialog window (but don't map it) */
void createmacrodialog (Tn5250Terminal * terminal, Tn5250Macro * macro);

/* Pop up the macro dialog window at the current mouse position */
void popmacrodialog (Tn5250Terminal * terminal, XEvent * report);

/* Draw the contents of the macro dialog window */
void drawmacrodialog (Tn5250Terminal * terminal, Tn5250Macro * macro);
