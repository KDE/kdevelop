/***************************************************************************
                                 defines.h
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

/**
 * in this file you´ll find some definations needed in many places of the
 * kdlgedit code.
*/


#ifndef KDLG_DEFINES_H
#define KDLG_DEFINES_H

#define KDLGEDIT_VERSION_STR "0.1"         // version of the dialog editor
#define KDEVELOP_VERSION_STR "0.3.1"       // version of kdevelop

#define MAX_WIDGETCOLS_PER_LINE 2          // defines up to which column in the property
					   // view a widget can be inserted

#define MAX_WIDGETS_PER_DIALOG 128         // maximum count of items/widgets in the main widget
#define MAX_ENTRYS_PER_WIDGET 64           // maximum count of child items/widgets per widget
                      	
// defines which type of input is allowed for an property
#define ALLOWED_STRING      1
#define ALLOWED_BOOL        2
#define ALLOWED_INT         3
#define ALLOWED_FILE        4
#define ALLOWED_COLOR       5
#define ALLOWED_FONT        6
#define ALLOWED_CONNECTIONS 7
#define ALLOWED_CURSOR      8
#define ALLOWED_MULTISTRING 9
#define ALLOWED_ORIENTATION 10
#define ALLOWED_VARNAME     11
#define ALLOWED_COMBOLIST   12

#define RULER_WIDTH  18                    // width of the vertical ruler
#define RULER_HEIGHT  18                   // height of the horizontal ruler

// defines the point the user has clicked on the widget in order to move/resize it
#define RESIZE_MOVE            0
#define RESIZE_TOP_LEFT        1
#define RESIZE_TOP_RIGHT       2
#define RESIZE_BOTTOM_LEFT     3
#define RESIZE_BOTTOM_RIGHT    4
#define RESIZE_MIDDLE_TOP      5
#define RESIZE_MIDDLE_BOTTOM   6
#define RESIZE_MIDDLE_LEFT     7
#define RESIZE_MIDDLE_RIGHT    8

// defines how many groups (i.e. general or appearance) are allowed in the property view
#define MAX_MAIN_ENTRYS 16

#endif





