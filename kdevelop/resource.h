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
#define ID_FILE_CLOSE_ALL           10065

#define ID_FILE_PRINT               10070

#define ID_FILE_QUIT                10080
#define ID_FILE_DELETE							10090

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

#define ID_EDIT_RUN_TO_CURSOR	      11136
#define ID_EDIT_STEP_OUT_OFF	      11137
#define ID_EDIT_ADD_WATCH_VARIABLE  11138

#define ID_EDIT_SPELLCHECK					11140

#define ID_EDIT_INDENT             	11091
#define ID_EDIT_UNINDENT            11092
#define ID_EDIT_COMMENT             11093
#define ID_EDIT_UNCOMMENT           11094

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

// The debugger entries
#define ID_DEBUG_RUN                14320
#define ID_DEBUG_STOP               14330
#define ID_DEBUG_STEP               14340
#define ID_DEBUG_NEXT               14350
#define ID_DEBUG_RESTART            14360
#define ID_DEBUG_MEMVIEW            14370
#define ID_DEBUG_BREAK_INTO         14380

// The debugger startups (Also ID_BUILD_DEBUG starts the debugger as well)
#define ID_DEBUG_SET_ARGS           14390
#define ID_DEBUG_CORE               14392
#define ID_DEBUG_NAMED_FILE         14394
#define ID_DEBUG_NORMAL             14396
#define ID_DEBUG_ATTACH             14398

///////////////////////////////////////////////////////////////////
// Classviewer popup entries 
#define ID_CV_WIZARD                    14510
#define ID_CV_GRAPHICAL_VIEW            14511

#define ID_CV_VIEW_DECLARATION          14512
#define ID_CV_VIEW_DEFINITION           14513

#define ID_CV_CLASS_TOOL                14514
#define ID_CV_CLASS_BASE_CLASSES        14515
#define ID_CV_CLASS_DERIVED_CLASSES     14516

#define ID_CV_FOLDER_NEW                14520
#define ID_CV_FOLDER_DELETE             14521

#define ID_CV_CLASS_DELETE              14522
#define ID_CV_VIEW_CLASS_DECLARATION    14523

#define ID_CV_METHOD_NEW                14524
#define ID_CV_METHOD_DELETE             14525

#define ID_CV_ATTRIBUTE_NEW             14526
#define ID_CV_ATTRIBUTE_DELETE          14527

#define ID_CV_IMPLEMENT_VIRTUAL         14530
#define ID_CV_ADD_SLOT_SIGNAL           14531
#define ID_CV_CLASSWIZARD               14532

#define	ID_CV_TOOLBAR_CLASS_CHOICE	14540
#define	ID_CV_TOOLBAR_METHOD_CHOICE	14541
#define	ID_CV_TOOLBAR_WIZARD_CHOICE	14542

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
#define ID_BOOKMARKS_TOGGLE					17010
#define ID_BOOKMARKS_CLEAR					17020
#define ID_BOOKMARKS_NEXT						17030
#define ID_BOOKMARKS_PREVIOUS				17040

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
#define ID_KDLG_BUILD_COMPLETE_GENERATE      20420

///////////////////////////////////////////////////////////////////
// Popup menu id's
#define ID_LFV_NEW_GROUP						30110
#define ID_LFV_REMOVE_GROUP					30120
#define ID_LFV_GROUP_PROP						30130
#define ID_LFV_SHOW_PATH_ITEM				30140
#define ID_RFV_SHOW_NONPRJFILES			30150

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
#define VAR 4

///////////////////////////////////////////////////////////////////
// tab-ID's for the messages_widget

#define MESSAGES 0
#define STDINSTDOUT 1
#define STDERR 2
#define BREAKPOINT 3
#define FRAME_STACK 4
#define DISASSEMBLE 5

///////////////////////////////////////////////////////////////////
// Toolbar ID's
#define ID_BROWSER_TOOLBAR 1
#define ID_KDLG_TOOLBAR 2
#define ID_DEBUG_TOOLBAR 3

///////////////////////////////////////////////////////////////////
// ID's for the statusbar
#define ID_STATUS_MSG               1001
#define ID_STATUS_CAPS              1007
#define ID_STATUS_INS_OVR           1002
#define ID_STATUS_LN_CLM            1003
#define ID_STATUS_EMPTY             1004
#define ID_STATUS_EMPTY_2           1005
#define ID_STATUS_PROGRESS          1006
#define ID_STATUS_DBG               1100

///////////////////////////////////////////////////////////////////
// ID's for the statusbar
#define ID_KDLG_STATUS_WIDGET       2001
#define ID_KDLG_STATUS_XY           2002
#define ID_KDLG_STATUS_WH           2003

///////////////////////////////////////////////////////////////////
// KEYBOARD ACCELERATORS
// NOTE:
//	CTRL+F1-F8 blocked by kpanel
//	CTRL+Alt+F1-Fx blocked to switch to konsoles
//	Alt+F1  K-Menu
//  Alt+F2  KDE execute
//	Alt+F3  Window menu

#define IDK_HELP_CONTENTS							Key_F1							// standard key

#define IDK_EDIT_GREP_IN_FILES				Key_F2
#define IDK_HELP_SEARCH_TEXT					SHIFT+Key_F2

#define IDK_EDIT_REPEAT_SEARCH				Key_F3

#define IDK_VIEW_NEXT_ERROR           Key_F4
#define IDK_VIEW_PREVIOUS_ERROR       SHIFT+Key_F4

#define IDK_BOOKMARKS_NEXT						Key_F5
#define IDK_BOOKMARKS_PREVIOUS				SHIFT+Key_F5

#define IDK_PROJECT_OPTIONS						Key_F7
#define IDK_PROJECT_FILE_PROPERTIES		SHIFT+Key_F7

#define IDK_BUILD_MAKE								Key_F8
#define IDK_BUILD_COMPILE_FILE	      SHIFT+Key_F8
#define IDK_BUILD_MAKE_WITH						ALT+Key_F8

#define IDK_BUILD_RUN									Key_F9
#define IDK_BUILD_RUN_WITH_ARGS       ALT+Key_F9

#define IDK_BUILD_STOP								Key_F10

#define IDK_VIEW_PREVIEW							CTRL+Key_Space

#define IDK_EDIT_SELECT_ALL						CTRL+Key_A
#define IDK_VIEW_OUTPUTVIEW	    			CTRL+Key_B
#define IDK_EDIT_COPY	       					CTRL+Key_C						// standard key
#define IDK_BOOKMARKS_CLEAR						CTRL+ALT+Key_C
#define IDK_TOOLS_KDEVKDLG            CTRL+Key_D
#define IDK_EDIT_SEARCH								CTRL+Key_F  					// standard key
#define IDK_EDIT_SEARCH_IN_FILES			CTRL+ALT+Key_F
#define IDK_VIEW_GOTO_LINE	    			CTRL+Key_G

#define IDK_EDIT_INDENT								CTRL+Key_I

#define IDK_TOGGLE_LAST        				CTRL+Key_L
#define IDK_EDIT_COMMENT							CTRL+Key_M
#define IDK_EDIT_UNCOMMENT						CTRL+ALT+Key_M
#define IDK_FILE_NEW	       					CTRL+Key_N						// standard key
#define IDK_FILE_OPEN	       					CTRL+Key_O						// standard key
#define IDK_FILE_PRINT         				CTRL+Key_P						// standard key
#define IDK_FILE_QUIT	       					CTRL+Key_Q						// standard key
#define IDK_EDIT_REPLACE							CTRL+Key_R						// standard key
#define IDK_FILE_SAVE	       					CTRL+Key_S						// standard key
#define IDK_VIEW_TREEVIEW	    				CTRL+Key_T
#define IDK_BOOKMARKS_TOGGLE					CTRL+ALT+Key_T
#define IDK_EDIT_UNINDENT							CTRL+Key_U
#define IDK_EDIT_PASTE								CTRL+Key_V						// standard key
#define IDK_FILE_CLOSE	       				CTRL+Key_W						// standard key
#define IDK_EDIT_CUT	       					CTRL+Key_X						// standard key
#define IDK_EDIT_REDO	       					CTRL+Key_Y						
#define IDK_EDIT_UNDO	       					CTRL+Key_Z						// standard key


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



