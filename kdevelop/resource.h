/***************************************************************************
                          resource.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Mon Oct  5 1998
                                           
    copyright            : (C) 1998 by Sandy Meier                     
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


///////////////////////////////////////////////////////////////////
// resource.h  -- contains macros used for commands


///////////////////////////////////////////////////////////////////
// COMMAND VALUES FOR MENUBAR AND TOOLBAR ENTRIES


///////////////////////////////////////////////////////////////////
// File-menu entries
#define ID_FILE_NEW_FILE            10010
#define ID_FILE_NEW_PROJECT         10020
#define ID_FILE_OPEN_FILE           10030
#define ID_FILE_OPEN_PROJECT        10040

#define ID_FILE_SAVE                10050
#define ID_FILE_SAVE_AS             10060
#define ID_FILE_SAVE_ALL            10065
#define ID_FILE_CLOSE               10070
#define ID_FILE_CLOSE_ALL           10075

#define ID_FILE_PRINT               10077

#define ID_FILE_QUIT                10080



///////////////////////////////////////////////////////////////////
// Edit-menu entries

#define ID_EDIT_UNDO                11005
#define ID_EDIT_REDO                11006

#define ID_EDIT_CUT                 11010
#define ID_EDIT_COPY                11020
#define ID_EDIT_PASTE               11030

#define ID_EDIT_INSERT_FILE         11050

#define ID_EDIT_SEARCH              11060
#define ID_EDIT_REPEAT_SEARCH       11070
#define ID_EDIT_REPLACE             11080
#define ID_EDIT_GOTO_LINE           11090

#define ID_EDIT_SELECT_ALL          11040
#define ID_EDIT_DESELECT_ALL        11041
#define ID_EDIT_INVERT_SELECTION    11042


///////////////////////////////////////////////////////////////////
// doc-menu entries                    
#define ID_DOC_BACK                 12010
#define ID_DOC_FORWARD              12020

#define ID_DOC_SEARCH_TEXT         12030

#define ID_DOC_QT_LIBRARY           12040
#define ID_DOC_KDE_CORE_LIBRARY     12050
#define ID_DOC_KDE_GUI_LIBRARY      12060
#define ID_DOC_KDE_KFILE_LIBRARY    12070
#define ID_DOC_KDE_HTML_LIBRARY     12080

#define ID_DOC_UPDATE_KDE_DOCUMENTATION 12085
#define ID_DOC_CREATE_SEARCHDATABASE 12087

#define ID_DOC_PROJECT_API_DOC      12090
#define ID_DOC_USER_MANUAL          12100

///////////////////////////////////////////////////////////////////
// build-menu entries 

#define ID_BUILD_RUN                13010
#define ID_BUILD_MAKE               13020
#define ID_BUILD_REBUILD_ALL        13030
#define ID_BUILD_CLEAN_REBUILD_ALL  13040
#define ID_BUILD_AUTOCONF           13045
#define ID_BUILD_STOP               13050

#define ID_BUILD_MAKE_PROJECT_API   13060
#define ID_BUILD_MAKE_USER_MANUAL   13070


///////////////////////////////////////////////////////////////////
// project-menu entries 

#define ID_PROJECT_ADD_FILE         14010
#define ID_PROJECT_ADD_FILE_NEW     14020
#define ID_PROJECT_ADD_FILE_EXIST   14030
#define ID_PROJECT_REMOVE_FILE      14040

#define ID_PROJECT_NEW_CLASS        14050

#define ID_PROJECT_FILE_PROPERTIES  14055
#define ID_PROJECT_OPTIONS          14060

///////////////////////////////////////////////////////////////////
// options-menu entries 

#define ID_OPTIONS_REFRESH          15010

#define ID_OPTIONS_STD_TOOLBAR      15020
#define ID_OPTIONS_BROWSER_TOOLBAR  15030
#define ID_OPTIONS_STATUSBAR        15040

#define ID_OPTIONS_TREEVIEW         15050
#define ID_OPTIONS_OUTPUTVIEW       15060

#define ID_OPTIONS_EDITOR           15065
#define ID_OPTIONS_EDITOR_COLORS           15066
#define ID_OPTIONS_SYNTAX_HIGHLIGHTING 15067
#define ID_OPTIONS_KDEVELOP  15070

///////////////////////////////////////////////////////////////////
// tools-menu entries 

#define ID_TOOLS_KICONEDIT          16010
#define ID_TOOLS_KDBG               16020

///////////////////////////////////////////////////////////////////
// help-menu-entries
#define ID_HELP_CONTENT             17010
#define ID_HELP_HOMEPAGE            17020
#define ID_HELP_ABOUT               17030

// for the setCurrentTab(int);

#define HEADER 0
#define CPP 1
#define TOOLS 3
#define BROWSER 2

#define ID_BROWSER_TOOLBAR 1



#define ID_STATUS_MSG               1001
#define ID_STATUS_INS_OVR           1002
#define ID_STATUS_LN_CLM            1003
#define ID_STATUS_EMPTY             1004
#define IDS_DEFAULT                 "Ready."


///////////////////////////////////////////////////////////////////
// MACROS FOR GENERATING THE SWITCH CONSTRUCTION OF THE statusCallback(int) METHOD

///////////////////////////////////////////////////////////////////
// Create class member in implementation for statusCallback
#define BEGIN_STATUS_MSG(class) void class::statusCallback(int id_){ switch (id_){

///////////////////////////////////////////////////////////////////
// Create cases for entries and connect them to change statusBar entry
#define ON_STATUS_MSG(id, message)     case id:  slotStatusHelpMsg(message);break;
 
///////////////////////////////////////////////////////////////////
// End class member in implementation for statusCallback
#define END_STATUS_MSG()         default: slotStatusMsg(IDS_DEFAULT); }}

