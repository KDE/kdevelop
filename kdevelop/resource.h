/***************************************************************************
                          resource.h  - 
                             -------------------                                         

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
#define ID_FILE_NEW			            10010
#define ID_FILE_OPEN    			      10020

#define ID_FILE_SAVE                10030
#define ID_FILE_SAVE_AS             10040
#define ID_FILE_SAVE_ALL            10050

#define ID_FILE_CLOSE               10060


#define ID_FILE_PRINT               10070

#define ID_FILE_QUIT                10080

///////////////////////////////////////////////////////////////////
// Edit-menu entries

#define ID_EDIT_UNDO                11010
#define ID_EDIT_REDO                11020

#define ID_EDIT_CUT                 11030
#define ID_EDIT_COPY                11040
#define ID_EDIT_PASTE               11050

#define ID_EDIT_INSERT_FILE         11060

#define ID_EDIT_SEARCH              11070
#define ID_EDIT_REPEAT_SEARCH       11080
#define ID_EDIT_REPLACE             11090
#define ID_EDIT_SEARCH_IN_FILES     11130

#define ID_EDIT_SPELLCHECK					11140

#define ID_EDIT_INDENT             	11091
#define ID_EDIT_UNINDENT            11092

#define ID_EDIT_SELECT_ALL          11100
#define ID_EDIT_DESELECT_ALL        11110
#define ID_EDIT_INVERT_SELECTION    11120

///////////////////////////////////////////////////////////////////
// View-menu entries

#define ID_VIEW_GOTO_LINE           12010

#define ID_VIEW_NEXT_ERROR          12015
#define ID_VIEW_PREVIOUS_ERROR      12017
#define ID_VIEW_TREEVIEW            12020
#define ID_VIEW_OUTPUTVIEW          12030

#define ID_VIEW_TOOLBAR             12040
#define ID_VIEW_BROWSER_TOOLBAR     12050
#define ID_VIEW_STATUSBAR           12060

#define ID_VIEW_REFRESH             12070
#define ID_VIEW_PREVIEW             12072

#define ID_VIEW_IN_KFM							12080

///////////////////////////////////////////////////////////////////
// Project-menu entries 

#define ID_PROJECT_KAPPWIZARD       13210
#define ID_PROJECT_OPEN			        13220
#define ID_PROJECT_OPEN_RECENT			13230
#define ID_PROJECT_CLOSE            13240
#define ID_PROJECT_NEW_CLASS        13250
#define ID_PROJECT_ADD_FILE_EXIST   13280
#define ID_PROJECT_ADD_NEW_TRANSLATION_FILE 13285
#define ID_PROJECT_REMOVE_FILE      13290
#define ID_PROJECT_COMPILE_FILE     13200


#define ID_PROJECT_WORKSPACES        13101
#define ID_PROJECT_WORKSPACES_1      13103
#define ID_PROJECT_WORKSPACES_2      13105
#define ID_PROJECT_WORKSPACES_3      13106
#define ID_PROJECT_FILE_PROPERTIES  13110
#define ID_PROJECT_OPTIONS          13120
#define ID_PROJECT_MESSAGES	          14120
#define ID_PROJECT_MAKE_PROJECT_API   14130
#define ID_PROJECT_MAKE_USER_MANUAL   14140

#define ID_PROJECT_MAKE_DISTRIBUTION  14200
#define ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ  14210


///////////////////////////////////////////////////////////////////
// Build-menu entries 
#define ID_BUILD_COMPILE_FILE       14010
#define ID_BUILD_MAKE               14020
#define ID_BUILD_MAKE_WITH          14030
#define ID_BUILD_REBUILD_ALL        14040
#define ID_BUILD_CLEAN_REBUILD_ALL  14050

#define ID_BUILD_STOP               14060
  
#define ID_BUILD_RUN                14070
#define ID_BUILD_RUN_WITH_ARGS			14075
#define ID_BUILD_DEBUG              14080

#define ID_BUILD_AUTOCONF           14090
#define ID_BUILD_CONFIGURE          14100
#define ID_BUILD_DISTCLEAN          14110




///////////////////////////////////////////////////////////////////
// Tools-menu entries 

#define ID_TOOLS_KDLGEDIT           15000
///////////////////////////////////////////////////////////////////
// Options-menu entries 

#define ID_OPTIONS_EDITOR          	      	     16010
#define ID_OPTIONS_EDITOR_COLORS           	     16020
#define ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS	     16030
#define ID_OPTIONS_SYNTAX_HIGHLIGHTING 	  	     16040

#define ID_OPTIONS_DOCBROWSER			               16050
#define ID_OPTIONS_TOOLS_CONFIG_DLG								16060	
#define ID_OPTIONS_SPELLCHECKER										16065
#define ID_OPTIONS_PRINT         16070
#define ID_OPTIONS_PRINT_A2PS          16071
#define ID_OPTIONS_PRINT_ENSCRIPT      16072

#define ID_OPTIONS_KDEVELOP            16080


///////////////////////////////////////////////////////////////////
// Bookmark-menu-entries
#define ID_BOOKMARKS_SET						17000
#define ID_BOOKMARKS_ADD						17010
#define ID_BOOKMARKS_CLEAR					17020

///////////////////////////////////////////////////////////////////
// Help-menu-entries
#define ID_HELP_BACK                 18010
#define ID_HELP_FORWARD              18020

#define ID_HELP_BROWSER_RELOAD			18022
#define ID_HELP_BROWSER_STOP				18023

#define ID_HELP_SEARCH_TEXT          18030
#define ID_HELP_SEARCH              18040

#define ID_HELP_CONTENTS             18050
#define ID_HELP_TUTORIAL						 18051
#define ID_HELP_TIP_OF_DAY						18052
#define ID_HELP_REFERENCE           18060

#define ID_HELP_QT_LIBRARY           18070
#define ID_HELP_KDE_CORE_LIBRARY     18080
#define ID_HELP_KDE_GUI_LIBRARY      18090
#define ID_HELP_KDE_KFILE_LIBRARY    18100
#define ID_HELP_KDE_HTML_LIBRARY     18110

#define ID_HELP_PROJECT_API			      18120
#define ID_HELP_USER_MANUAL          18130
#define ID_HELP_HOMEPAGE            18140
#define ID_HELP_BUG_REPORT          18145
#define ID_HELP_DLGNOTES            18149
#define ID_HELP_ABOUT               18150

#define ID_HELP_WHATS_THIS     			18160

///////////////////////////////////////////////////////////////////
// KDlg-File-menu-entries
#define ID_KDLG_FILE_CLOSE          20030
#define ID_KDLG_FILE_SAVE           20040
#define ID_KDLG_FILE_SAVE_AS           20050
///////////////////////////////////////////////////////////////////
// KDlg-Edit-menu-entries
#define ID_KDLG_EDIT_UNDO           20110
#define ID_KDLG_EDIT_REDO           20120

#define ID_KDLG_EDIT_CUT            20130
#define ID_KDLG_EDIT_COPY           20140
#define ID_KDLG_EDIT_PASTE          20150
#define ID_KDLG_EDIT_DELETE         20155

///////////////////////////////////////////////////////////////////
// KDlg-View-menu-entries
#define ID_KDLG_VIEW_PROPVIEW       20220

#define ID_KDLG_VIEW_TOOLBAR        20230
#define ID_KDLG_VIEW_STATUSBAR      20240

#define ID_KDLG_VIEW_REFRESH        20250
#define ID_KDLG_VIEW_GRID           20255

///////////////////////////////////////////////////////////////////
// KDlg-Tools-menu-entries
#define ID_KDLG_TOOLS_KDEVELOP      11

///////////////////////////////////////////////////////////////////
// KDlg-Build-menu-entries
#define ID_KDLG_BUILD_GENERATE      20410

///////////////////////////////////////////////////////////////////
// tab-ID's for the s_tab_view
#define HEADER  0
#define CPP 	1
#define BROWSER 2
#define TOOLS 	3

///////////////////////////////////////////////////////////////////
// tab-ID's for the t_tab_view
#define CV  0
#define LFV 1
#define RFV 2
#define DOC 3

///////////////////////////////////////////////////////////////////
// tab-ID's for the messages_widget

#define MESSAGES 0
#define STDINSTDOUT 1
#define STDERR 2

///////////////////////////////////////////////////////////////////
// Toolbar ID's
#define ID_BROWSER_TOOLBAR 1
#define ID_KDLG_TOOLBAR 2


///////////////////////////////////////////////////////////////////
// ID's for the statusbar
#define ID_STATUS_MSG               1001
#define ID_STATUS_CAPS              1007
#define ID_STATUS_INS_OVR           1002
#define ID_STATUS_LN_CLM            1003
#define ID_STATUS_EMPTY             1004
#define ID_STATUS_EMPTY_2           1005
#define ID_STATUS_PROGRESS          1006

///////////////////////////////////////////////////////////////////
// ID's for the statusbar
#define ID_KDLG_STATUS_WIDGET       2001
#define ID_KDLG_STATUS_XY           2002
#define ID_KDLG_STATUS_WH           2003

///////////////////////////////////////////////////////////////////
// KEY-VALUES

///////////////////////////////////////////////////////////////////
// File-menu entries
#define IDK_FILE_NEW	       					CTRL+Key_N
#define IDK_FILE_OPEN	       					CTRL+Key_O
#define IDK_FILE_SAVE	       					CTRL+Key_S
#define IDK_FILE_PRINT         				CTRL+Key_P
#define IDK_FILE_CLOSE	       				CTRL+Key_W
#define IDK_FILE_QUIT	       					CTRL+Key_Q

///////////////////////////////////////////////////////////////////
// Edit-menu entries
#define IDK_EDIT_UNDO	       					CTRL+Key_Z
#define IDK_EDIT_REDO	       					CTRL+Key_Y

#define IDK_EDIT_CUT	       					CTRL+Key_X
#define IDK_EDIT_COPY	       					CTRL+Key_C
#define IDK_EDIT_PASTE								CTRL+Key_V

#define IDK_EDIT_REPEAT_SEARCH				Key_F3
#define IDK_EDIT_SEARCH_IN_FILES			CTRL+ALT+Key_F

#define IDK_EDIT_INDENT								CTRL+Key_I
#define IDK_EDIT_UNINDENT							CTRL+Key_U

///////////////////////////////////////////////////////////////////
// View-menu entries
#define IDK_VIEW_GOTO_LINE	    			CTRL+Key_L
#define IDK_VIEW_NEXT_ERROR           Key_F4
#define IDK_VIEW_PREVIOUS_ERROR       SHIFT+Key_F4
#define IDK_VIEW_TREEVIEW	    				CTRL+Key_T
#define IDK_VIEW_OUTPUTVIEW	    			CTRL+Key_G
#define IDK_VIEW_PREVIEW							CTRL+Key_Space

///////////////////////////////////////////////////////////////////
// Project-menu entries 

///////////////////////////////////////////////////////////////////
// Build-menu entries 
#define IDK_BUILD_COMPILE_FILE	      CTRL+Key_F10
#define IDK_BUILD_MAKE								Key_F9
#define IDK_BUILD_MAKE_WITH						ALT+Key_F9
#define IDK_BUILD_RUN									CTRL+Key_F9
#define IDK_BUILD_RUN_WITH_ARGS       ALT+Key_F10

///////////////////////////////////////////////////////////////////
// Tools-menu entries 
#define IDK_TOOLS_KDEVKDLG            CTRL+Key_D
///////////////////////////////////////////////////////////////////
// Options-menu entries 

///////////////////////////////////////////////////////////////////
// Bookmarks-menu entries
#define IDK_BOOKMARKS_ADD							CTRL+ALT+Key_A
#define IDK_BOOKMARKS_CLEAR						CTRL+ALT+Key_C

///////////////////////////////////////////////////////////////////
// Help-menu-entries
#define IDK_HELP_SEARCH_TEXT					Key_F2				
#define IDK_HELP_CONTENTS							Key_F1

#define IDK_TOGGLE_LAST        				CTRL+Key_B


///////////////////////////////////////////////////////////////////
// MACROS FOR THE CONNECT OF YOUR SIGNALS TO CORRESPONDENT SLOTS
// IN YOUR MAIN IMPLEMENTATION OF MENUBAR AND TOOLBAR
///////////////////////////////////////////////////////////////////
// MENU CONNECTS
#define CONNECT_CMD(submenu)           connect(submenu, SIGNAL(activated(int)), SLOT(commandCallback(int)));connect(submenu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)))


///////////////////////////////////////////////////////////////////
// TOOLBAR CONNECT
#define CONNECT_TOOLBAR(ToolBar)        connect(ToolBar, SIGNAL(clicked(int)),SLOT(commandCallback(int)));connect(ToolBar, SIGNAL(highlighted(int,bool)), SLOT(statusCallback(int)))


///////////////////////////////////////////////////////////////////
// MACROS FOR GENERATING THE SWITCH CONSTRUCTION OF THE statusCallback(int) METHOD

///////////////////////////////////////////////////////////////////
// Create cases for entries and connect them to change statusBar entry
#define ON_STATUS_MSG(id, message)     case id:  slotStatusHelpMsg(message);break;

























