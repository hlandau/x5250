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

/* Parses the command to get the X defaults parts.  The rest of the command
 * line will get parsed later.
 */
void parse_command_line_X ();

/* Merges the X resource databases.
 */
void merge_db_X ();

/* Gets the user defaults from the X resource database.
 */
void get_resources_X (Tn5250Terminal * terminal);
