/***************************************************************************
                          ckdevelop_whatsthis.cpp  -  description                              
                             -------------------                                         
    begin                : Mon Feb 15 1999                                           
    copyright            : (C) 1999 by Ralf Nolden                          
    email                : Ralf.Nolden@post.rwth-aachen.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <ktoolbarbutton.h>
#include "ckdevelop.h"
#include "cclassview.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "makeview.h"
#include "ceditwidget.h"
 
void CKDevelop::initWhatsThis(){
    QWhatsThis::add(toolBar()->getButton(ID_PROJECT_OPEN), i18n("Open project\n\n"
																															"Shows the open project dialog "
																															"to select a project to be opened"));
																															

																																	

	
	QWhatsThis::add(toolBar()->getButton(ID_FILE_OPEN),i18n("Open file\n\n"
																													"Shows the Open file dialog to "
																													"select a file to be opened. Holding "
																													"the button pressed will show a popup "
																													"menu containing all filenames of your "
																													"project's sources and header files. "
																													"Selecting a filename on the menu will "
																													"then open the file according to the "
																													"file-type."));
	
													

	QWhatsThis::add(toolBar()->getButton(ID_FILE_SAVE),i18n("Save file\n\n"
																													"Saves the file opened in the actual "
																													"editing view"));
		
	
	
	QWhatsThis::add(toolBar()->getButton(ID_FILE_PRINT),i18n("Print\n\n"
	                                                        "Opens the printing dialog. There, you can "
	                                                        "configure which printing program you wish "
	                                                        "to use, either a2ps or ensrcipt, and print "
	                                                        "your project files."));
	
	QWhatsThis::add(toolBar()->getButton(ID_EDIT_UNDO),i18n("Undo\n\n"
	                                                        "Reverts the last editing step."));
	
	
	
	QWhatsThis::add(toolBar()->getButton(ID_EDIT_REDO),i18n("Redo\n\n"
	                                                        "If an editing step was undone, redo "
	                                                        "lets you do this step again."));
	
	

	QWhatsThis::add(toolBar()->getButton(ID_EDIT_CUT),i18n("Cut\n\n"
	                                                        "Cuts out the selected text and copies "
	                                                        "it to the system clipboard."));	                                                        	
	
	                                                      	
	
	QWhatsThis::add(toolBar()->getButton(ID_EDIT_COPY),i18n("Copy\n\n"
	                                                        "Copies the selected text into the "
	                                                        "system clipboard."));
		
	
		
	QWhatsThis::add(toolBar()->getButton(ID_EDIT_PASTE),i18n("Paste\n\n"
	                                                          "Inserts the contents of the "
	                                                          "system clipboard at the current "
	                                                          "cursor position. "));
	
	
	
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_COMPILE_FILE),i18n("Compile file\n\n"
	                                                                  "Only compile the file opened in "
	                                                                  "the C/C++ Files- window. The output "
	                                                                  "is shown in the output window. If "
	                                                                  "errors occur, clicking on the error line "
	                                                                  "causes the file window to show you the "
	                                                                  "line the error occured."));
	/*	QWhatsThis::add(toolBar()->getButton(ID_KDLG_BUILD_GENERATE),i18n("Generate files\n\n"
																																		"This will actually generate the source "
																																		"files for the current dialog opened. "
																																		"The sources will be added to the project "
																																		"if one is opened, otherwise you will be "
																																		"asked to select the output directory for "
																																		"the generated files."));
	*/
	QString build_make_str=i18n("Make\n\n"
								"Invokes the make-command set in the "
	              "options-menu for the current project "
	              "after saving all files. "
	              "This will compile all changed sources "
	              "since the last compilation was invoked.\n"
	              "The output window opens to show compiler "
	              "messages. If errors occur, clicking on the "
	              "error line will open the file where the "
	              "error was found and sets the cursor to the "
	              "error line.");																																		
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_MAKE),build_make_str);

		
	QString build_rebuild_all_str=i18n("Rebuild all\n\n"
								"After saving all files, rebuild all "
								"invokes the make-command set with the "
								"clean-option to remove all object files. "
								"Then, configure creates new Makefiles and "
								"the make-command will rebuild the project.");	
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_REBUILD_ALL),build_rebuild_all_str);


	QString build_run_str=i18n("Execute\n\n"
								"After saving all files,the make-command is "
								"called to build the project. Then the binary "
								"is executed out of the project directory.\n"
	           		"Be aware that this function is only valid for "
	              "programs and that references to e.g. pixmaps "
	              "or html help files that are supposed to be "
	              "installed will cause some strange behavoir "
	              "like testing the helpmenu will open an error "
	              "message that the index.html file is not found.");	
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_RUN),build_run_str);

	
	QString build_debug_str=i18n("Debug program\n\n"
								"Opens KDbg in the tools window with your actual"
	       				"program and allows you to execute it step by step"
	           		"by setting breakpoints in the sourcecode."
	             	"Note that you have to open your sourcecodes in KDbg"
	              "to access it.");
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_DEBUG),build_debug_str);

	
	QString build_stop_str=i18n("Stop\n\n"
								"If activated, the stop-command will interrupt "
	       				"the active process. This affects make-commands "
	           		"as well as documentation generation.");	
	QWhatsThis::add(toolBar()->getButton(ID_BUILD_STOP),build_stop_str);


	
	
	QString output=i18n("Output-View\n\n"
											"Enables/ disables the output window. The "																
											"state of the window is displayed by the "
											"toggle button: if the button is pressed, "
											"the window is open, otherwise closed.");
											
	QWhatsThis::add(toolBar()->getButton(ID_VIEW_OUTPUTVIEW),output);

	
	QString treeview=i18n("Tree-View\n\n"
											"Enables/ disables the tree window. The "																
											"state of the window is displayed by the "
											"toggle button: if the button is pressed, "
											"the window is open, otherwise closed.");
	QWhatsThis::add(toolBar()->getButton(ID_VIEW_TREEVIEW),treeview);

											
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE),i18n("Classes\n\n"
	                                                    "Choosing a class will switch to the header "
	                                                    "file where the class is declarated."));
	
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE),i18n("Methods\n\n"
	                                                    "Allows selecting a method of the current "
	                                                    "selected class. This will switch to the "
	                                                    "implementation of the chosen class."));
	
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_BACK),i18n("Documentation-Back\n\n"
	                                                    "This opens the previously visited page in the "
	                                                    "browser window. Holding the button pressed down "
	                                                    "will open a popup menu showing the browser history. "
	                                                    "Selecting an entry will open the according page in "
	                                                    "the browser window."));
	
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_FORWARD),i18n("Documentation-Forward\n\n"
	                                                    "This opens the next page in the history list. "
	                                                    "Holding the button pressed will show you a popup "
	                                                    "menu to browse visited pages forward."));
	
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_SEARCH_TEXT),i18n("Search Marked Text\n\n"
	                                                    "Uses the searchdatabase to look for the selected "
	                                                    "text string and opens a summary page of found "
	                                                    "references for the selection in the browser window. "
	                                                    "There, you can select a documentation page of your "
	                                                    "choice and open it by following the link."));			
	
	QWhatsThis::add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_SEARCH),i18n("Search for Help on...\n\n"
		                                                  "Opens the Search for Help on... dialog. There, you can "
		                                                  "search for a keyword in the documentation directly "
		                                                  "using the search database."));
	QWhatsThis::add(class_tree, i18n("Class Viewer\n\n"
	                                  "The class viewer shows all classes, methods and variables "
	                                  "of the current project files and allows switching to declarations "
	                                  "and implementations. The right button popup-menu allows more specialized "
	                                  "functionality."));
	
	QWhatsThis::add(log_file_tree, i18n("Logical File Viewer\n\n"
	                                  "The logical file viewer separates your project files into "
	                                  "logical groups to allow easier access e.g.to header and "
	                                  "implementation files.The right button popup-menu allows more specialized "
	                                  "functionality."));
	
	QWhatsThis::add(real_file_tree, i18n("Real File Viewer\n\n"
	                                  "The real file viewer shows the project directory in "
	                                  "it's real structure and gives access to all files."));
	
	QWhatsThis::add(doc_tree, i18n("Documentation Tree\n\n"
	                                  "The documentation tree gives access to library "
	                                  "documentation and the KDevelop manuals. Can be "
	                                  "configured individually by right-button menu."));
	


#warning FIXME MDI stuff														
	// QWhatsThis::add(header_widget, i18n("Header/Resource Files\n\n"
// 	                                  "This window shows header and all other project files "
// 	                                  "except for source files. The statusbar shows the "
// 	                                  "status of the insert/overwrite key as well as the "
// 	                                  "cursor position by line and column."));
	
// 	QWhatsThis::add(cpp_widget, i18n("C/C++ Files\n\n"
// 	                                  "This window shows only source files. The statusbar shows "
// 	                                  "the status of the insert/overwrite key as well as the "
// 	                                  "cursor position by line and column."));
	
	QWhatsThis::add(browser_widget, i18n("Documentation Browser\n\n"
	                                  "The documentation browser window shows the online-"
	                                  "documentation provided with kdevelop as well as "
	                                  "library class documentation created. Use the documentation "
	                                  "tree to switch between various parts of the documentation."));	
	
	QWhatsThis::add(messages_widget, i18n("Messages\n\n"
	                                  "The messages window shows the output of the compiler and "
	                                  "used utilities like kdoc reference documentation. "
	                                  "For compiler error messages, click on the error message. "
	                                  "This will automatically open the source file and set the "
	                                  "cursor to the line that caused the compiler error/warning. "));

	// this doesn't work because of the ClickFocus policy of the stdin/stdout widget. NoFocus functions but
	// disables the functionality of the widget...:-(                                  	
	/*	QWhatsThis::add(stdin_stdout_widget, i18n("StdOut\n\n"
		"The Standard Input/Standard Output window is a replacement "
		"for terminal-based application communication. Running "
		"terminal applications are using this instead of a terminal window."));

	QWhatsThis::add(stderr_widget, i18n("StdErr\n\n""The Standard Error window displays messages of running applications ""using the cerr function to debug."));
	*/
                                  																												
}
















