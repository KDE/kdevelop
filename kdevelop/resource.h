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

#define ID_EDIT_INDENT             	11091
#define ID_EDIT_UNINDENT            11092

#define ID_EDIT_SELECT_ALL          11100
#define ID_EDIT_DESELECT_ALL        11110
#define ID_EDIT_INVERT_SELECTION    11120

///////////////////////////////////////////////////////////////////
// View-menu entries

#define ID_VIEW_GOTO_LINE           12010
#define ID_VIEW_GOTO_BOOKMARK       12011
#define ID_VIEW_BOOKMARK_ADD        12012
#define ID_VIEW_BOOKMARKS_CLEAR     12013

#define ID_VIEW_TREEVIEW            12020
#define ID_VIEW_OUTPUTVIEW          12030

#define ID_VIEW_TOOLBAR             12040
#define ID_VIEW_BROWSER_TOOLBAR     12050
#define ID_VIEW_STATUSBAR           12060

#define ID_VIEW_REFRESH             12070

///////////////////////////////////////////////////////////////////
// Project-menu entries 

#define ID_PROJECT_KAPPWIZARD       13010
#define ID_PROJECT_NEW              13020
#define ID_PROJECT_OPEN			        13030
#define ID_PROJECT_CLOSE            13040
#define ID_PROJECT_NEW_CLASS        13050
#define ID_PROJECT_ADD_FILE         13060
#define ID_PROJECT_ADD_FILE_NEW     13070
#define ID_PROJECT_ADD_FILE_EXIST   13080
#define ID_PROJECT_REMOVE_FILE      13090
#define ID_PROJECT_COMPILE_FILE     13100


#define ID_PROJECT_WORKSPACES        13101
#define ID_PROJECT_WORKSPACES_1      13103
#define ID_PROJECT_WORKSPACES_2      13105
#define ID_PROJECT_WORKSPACES_3      13106
#define ID_PROJECT_FILE_PROPERTIES  13110
#define ID_PROJECT_OPTIONS          13120


///////////////////////////////////////////////////////////////////
// Build-menu entries 
#define ID_BUILD_COMPILE_FILE       14010
#define ID_BUILD_MAKE               14020
#define ID_BUILD_MAKE_WITH          14030
#define ID_BUILD_REBUILD_ALL        14040
#define ID_BUILD_CLEAN_REBUILD_ALL  14050

#define ID_BUILD_STOP               14060
  
#define ID_BUILD_RUN                14070
#define ID_BUILD_DEBUG              14080

#define ID_BUILD_AUTOCONF           14090
#define ID_BUILD_CONFIGURE          14100
#define ID_BUILD_DISTCLEAN          14110

#define ID_BUILD_MESSAGES	          14120
#define ID_BUILD_MAKE_PROJECT_API   14130
#define ID_BUILD_MAKE_USER_MANUAL   14140


///////////////////////////////////////////////////////////////////
// Tools-menu entries 

#define ID_TOOLS_KICONEDIT          15010
#define ID_TOOLS_KDBG               15020
#define ID_TOOLS_KTRANSLATOR        15030
                                     
///////////////////////////////////////////////////////////////////
// Options-menu entries 

#define ID_OPTIONS_EDITOR          	      	     16010
#define ID_OPTIONS_EDITOR_COLORS           	     16020
#define ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS	     16025
#define ID_OPTIONS_SYNTAX_HIGHLIGHTING 	  	     16030

#define ID_OPTIONS_DOCBROWSER			               16050

#define ID_OPTIONS_KEYS          16035

#define ID_OPTIONS_PRINT         16036
#define ID_OPTIONS_PRINT_A2PS          16037
#define ID_OPTIONS_PRINT_ENSCRIPT      16038

#define ID_OPTIONS_KDEVELOP            16040




///////////////////////////////////////////////////////////////////
// Help-menu-entries
#define ID_HELP_BACK                 17010
#define ID_HELP_FORWARD              17020

#define ID_HELP_SEARCH_TEXT          17030
#define ID_HELP_SEARCH              18005

#define ID_HELP_CONTENTS             18010
#define ID_HELP_REFERENCE           18007
#define ID_HELP_QT_LIBRARY           17040
#define ID_HELP_KDE_CORE_LIBRARY     17050
#define ID_HELP_KDE_GUI_LIBRARY      17060
#define ID_HELP_KDE_KFILE_LIBRARY    17070
#define ID_HELP_KDE_HTML_LIBRARY     17080

#define ID_HELP_PROJECT_API			      17090
#define ID_HELP_USER_MANUAL          17100
#define ID_HELP_HOMEPAGE            18020
#define ID_HELP_ABOUT               18030

#define ID_HELP_WHATS_THIS     			18040


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

#define ID_BROWSER_TOOLBAR 1


///////////////////////////////////////////////////////////////////
// ID's for the statusbar
#define ID_STATUS_MSG               1001
#define ID_STATUS_CAPS              1007
#define ID_STATUS_INS_OVR           1002
#define ID_STATUS_LN_CLM            1003
#define ID_STATUS_EMPTY             1004
#define ID_STATUS_EMPTY_2           1005
#define ID_STATUS_PROGRESS          1006
#define IDS_DEFAULT                 i18n("Ready.")

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

#define IDK_EDIT_SEARCH								CTRL+Key_F
#define IDK_EDIT_REPEAT_SEARCH				Key_F3
#define IDK_EDIT_REPLACE							CTRL+Key_R

#define IDK_EDIT_INDENT								CTRL+Key_I
#define IDK_EDIT_UNINDENT							CTRL+Key_U

///////////////////////////////////////////////////////////////////
// View-menu entries
#define IDK_VIEW_GOTO_LINE	    			CTRL+Key_L
#define IDK_VIEW_TREEVIEW	    				CTRL+Key_T
#define IDK_VIEW_OUTPUTVIEW	    			CTRL+Key_G
#define IDK_VIEW_STATUSBAR          	CTRL+Key_B			

///////////////////////////////////////////////////////////////////
// Project-menu entries 

///////////////////////////////////////////////////////////////////
// Build-menu entries 
#define IDK_BUILD_COMPILE_FILE	      CTRL+Key_F10
#define IDK_BUILD_MAKE								Key_F9
#define IDK_BUILD_MAKE_WITH						ALT+Key_F9
#define IDK_BUILD_RUN									CTRL+Key_F9

///////////////////////////////////////////////////////////////////
// Tools-menu entries 

///////////////////////////////////////////////////////////////////
// Options-menu entries 


///////////////////////////////////////////////////////////////////
// Help-menu-entries
#define IDK_HELP_SEARCH_TEXT					Key_F2				
#define IDK_HELP_CONTENTS							Key_F1

#define IDK_SHOW_HEADER        				CTRL+Key_1
#define IDK_SHOW_C	       						CTRL+Key_2
#define IDK_SHOW_HELP	       					CTRL+Key_3
#define IDK_SHOW_TOOLS	       				CTRL+Key_4
#define IDK_TOGGLE_LAST        				CTRL+Key_Tab


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
// MACROS FOR GENERATING THE SWITCH CONSTRUCTION OF THE commandCallback(int) METHOD

///////////////////////////////////////////////////////////////////
// Create class member in implementation for commandCallback
#define BEGIN_CMD(class)         void class::commandCallback(int id_){ switch (id_){

///////////////////////////////////////////////////////////////////
// Create cases for entries and connect them with their functions
#define ON_CMD(id, cmd_class_function, message)   case id:slotStatusMsg(message);cmd_class_function ;break;

///////////////////////////////////////////////////////////////////
// End class member in implementation for commandCallback
#define END_CMD()      }}



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
#define END_STATUS_MSG()         default: slotStatusMsg(IDS_DEFAULT);}}




