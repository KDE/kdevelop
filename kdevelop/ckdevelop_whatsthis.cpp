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

 
#include "ckdevelop.h"
#include "cclassview.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
 
void CKDevelop::initWhatsThis(){
	whats_this->add(toolBar()->getButton(ID_PROJECT_OPEN), i18n("Open project\n\n"
																															"Shows the open project dialog "
																															"to select a project to be opened"));
																															
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_PROJECT_OPEN), i18n("Open project\n\n"
																															"Shows the open project dialog "
																															"to select a project to be opened"));
																																	

	
	whats_this->add(toolBar()->getButton(ID_FILE_OPEN),i18n("Open file\n\n"
																													"Shows the Open file dialog to "
																													"select a file to be opened. Holding "
																													"the button pressed will show a popup "
																													"menu containing all filenames of your "
																													"project's sources and header files. "
																													"Selecting a filename on the menu will "
																													"then open the file according to the "
																													"file-type."));
	
													

	whats_this->add(toolBar()->getButton(ID_FILE_SAVE),i18n("Save file\n\n"
																													"Saves the file opened in the actual "
																													"editing view"));
		
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_FILE_SAVE),i18n("Save file\n\n"
																													"Saves the file opened in the actual "
																													"editing view"));
	
	whats_this->add(toolBar()->getButton(ID_FILE_PRINT),i18n("Print\n\n"
	                                                        "Opens the printing dialog. There, you can "
	                                                        "configure which printing program you wish "
	                                                        "to use, either a2ps or ensrcipt, and print "
	                                                        "your project files."));
	
	whats_this->add(toolBar()->getButton(ID_EDIT_UNDO),i18n("Undo\n\n"
	                                                        "Reverts the last editing step."));
	
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_EDIT_UNDO),i18n("Undo\n\n"
	                                                        "Reverts the last editing step."));
	
	whats_this->add(toolBar()->getButton(ID_EDIT_REDO),i18n("Redo\n\n"
	                                                        "If an editing step was undone, redo "
	                                                        "lets you do this step again."));
	
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_EDIT_REDO),i18n("Redo\n\n"
	                                                        "If an editing step was undone, redo "
	                                                        "lets you do this step again."));

	whats_this->add(toolBar()->getButton(ID_EDIT_CUT),i18n("Cut\n\n"
	                                                        "Cuts out the selected text and copies "
	                                                        "it to the system clipboard."));	                                                        	
	
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_EDIT_CUT),i18n("Cut\n\n"
	                                                        "Cuts out the selected widget and copies "
	                                                        "it to the system clipboard."));	                                                        	
	
	whats_this->add(toolBar()->getButton(ID_EDIT_COPY),i18n("Copy\n\n"
	                                                        "Copies the selected text into the "
	                                                        "system clipboard."));
		
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_EDIT_COPY),i18n("Copy\n\n"
	                                                        "Copies the selected widget into the "
	                                                        "system clipboard."));
		
	whats_this->add(toolBar()->getButton(ID_EDIT_PASTE),i18n("Paste\n\n"
	                                                          "Inserts the contents of the "
	                                                          "system clipboard at the current "
	                                                          "cursor position. "));
	
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_EDIT_PASTE),i18n("Paste\n\n"
	                                                          "Inserts the contents of the "
	                                                          "system clipboard at the current "
	                                                          "cursor position. "));
	
	whats_this->add(toolBar()->getButton(ID_BUILD_COMPILE_FILE),i18n("Compile file\n\n"
	                                                                  "Only compile the file opened in "
	                                                                  "the C/C++ Files- window. The output "
	                                                                  "is shown in the output window. If "
	                                                                  "errors occur, clicking on the error line "
	                                                                  "causes the file window to show you the "
	                                                                  "line the error occured."));
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_BUILD_GENERATE),i18n("Generate files\n\n"
																																		"This will actually generate the source "
																																		"files for the current dialog opened. "
																																		"The sources will be added to the project "
																																		"if one is opened, otherwise you will be "
																																		"asked to select the output directory for "
																																		"the generated files."));
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
	whats_this->add(toolBar()->getButton(ID_BUILD_MAKE),build_make_str);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_BUILD_MAKE),build_make_str);
		
	QString build_rebuild_all_str=i18n("Rebuild all\n\n"
								"After saving all files, rebuild all "
								"invokes the make-command set with the "
								"clean-option to remove all object files. "
								"Then, configure creates new Makefiles and "
								"the make-command will rebuild the project.");	
	whats_this->add(toolBar()->getButton(ID_BUILD_REBUILD_ALL),build_rebuild_all_str);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_BUILD_REBUILD_ALL),build_rebuild_all_str);

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
	whats_this->add(toolBar()->getButton(ID_BUILD_RUN),build_run_str);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_BUILD_RUN),build_run_str);

	QString build_debug_str=i18n("Debug program\n\n"
								"Runs your program in the debugger you have chosen"
                "(by default the internal debugger is used)"
	       				"All debuggers should allow you to execute your program"
                "step by step by setting breakpoints in the sourcecode." );
	whats_this->add(toolBar()->getButton(ID_BUILD_DEBUG),build_debug_str);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_BUILD_DEBUG),build_debug_str);

	QString build_stop_str=i18n("Stop\n\n"
								"If activated, the stop-command will interrupt "
	       				"the active process. This affects make-commands "
	           		"as well as documentation generation.");	
	whats_this->add(toolBar()->getButton(ID_BUILD_STOP),build_stop_str);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_BUILD_STOP),build_stop_str);

	whats_this->add(toolBar()->getButton(ID_TOOLS_KDLGEDIT),i18n("Dialogeditor\n\n"
																											"This will switch to the internal dialog "
																											"editor of the KDevelop IDE. "
																											"There, you can also switch back to the "
																											"editing mode by choosing the Sourcecode editor "
																											"button or Sourcecode-editor from the View-menu."));
																											
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_KDLG_TOOLS_KDEVELOP),i18n("Sourcecode Editor\n\n"
																											"This will switch to the sourcecode editor "
																											"providing the usual IDE interface. You can "
																											"switch back to the dialogeditor by choosing "
																											"the Dialogeditor button in the KDevelop toolbar "
																											"or by choosing Dialogeditor in the View-menu."));
	
	QString output=i18n("Output-View\n\n"
											"Enables/ disables the output window. The "																
											"state of the window is displayed by the "
											"toggle button: if the button is pressed, "
											"the window is open, otherwise closed.");
											
	whats_this->add(toolBar()->getButton(ID_VIEW_OUTPUTVIEW),output);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_VIEW_OUTPUTVIEW),output);
	
	QString treeview=i18n("Tree-View\n\n"
											"Enables/ disables the tree window. The "																
											"state of the window is displayed by the "
											"toggle button: if the button is pressed, "
											"the window is open, otherwise closed.");
	whats_this->add(toolBar()->getButton(ID_VIEW_TREEVIEW),treeview);
	whats_this->add(toolBar(ID_KDLG_TOOLBAR)->getButton(ID_VIEW_TREEVIEW),treeview);
											
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE),i18n("Classes\n\n"
	                                                    "Choosing a class will switch to the header "
	                                                    "file where the class is declared."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE),i18n("Methods\n\n"
	                                                    "Allows selecting a method of the current "
	                                                    "selected class. This will switch to the "
	                                                    "implementation of the chosen class."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_BACK),i18n("Documentation-Back\n\n"
	                                                    "This opens the previously visited page in the "
	                                                    "browser window. Holding the button pressed down "
	                                                    "will open a popup menu showing the browser history. "
	                                                    "Selecting an entry will open the according page in "
	                                                    "the browser window."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_FORWARD),i18n("Documentation-Forward\n\n"
	                                                    "This opens the next page in the history list. "
	                                                    "Holding the button pressed will show you a popup "
	                                                    "menu to browse visited pages forward."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_SEARCH_TEXT),i18n("Search Marked Text\n\n"
	                                                    "Uses the searchdatabase to look for the selected "
	                                                    "text string and opens a summary page of found "
	                                                    "references for the selection in the browser window. "
	                                                    "There, you can select a documentation page of your "
	                                                    "choice and open it by following the link."));			
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_HELP_SEARCH),i18n("Search for Help on...\n\n"
		                                                  "Opens the Search for Help on... dialog. There, you can "
		                                                  "search for a keyword in the documentation directly "
		                                                  "using the search database."));
	whats_this->add(class_tree, i18n("Class Viewer\n\n"
	                                  "The class viewer shows all classes, methods and variables "
	                                  "of the current project files and allows switching to declarations "
	                                  "and implementations. The right button popup-menu allows more specialized "
	                                  "functionality."));
	
	whats_this->add(log_file_tree, i18n("Logical File Viewer\n\n"
	                                  "The logical file viewer separates your project files into "
	                                  "logical groups to allow easier access e.g.to header and "
	                                  "implementation files.The right button popup-menu allows more specialized "
	                                  "functionality."));
	
	whats_this->add(real_file_tree, i18n("Real File Viewer\n\n"
	                                  "The real file viewer shows the project directory in "
	                                  "it's real structure and gives access to all files."));
	
	whats_this->add(doc_tree, i18n("Documentation Tree\n\n"
	                                  "The documentation tree gives access to library "
	                                  "documentation and the KDevelop manuals. Can be "
	                                  "configured individually by right-button menu."));
	
	whats_this->add((QWidget*)kdlg_widgets_view, i18n("Widgets\n\n"
																		"Here, you can select a widget item to add to "
																		"the dialog you're working on. More information "
																		"about the widget items can be found by What's this "
																		"help on the buttons or in the KDevelop documentation. "));
	
	whats_this->add((QWidget*)kdlg_dialogs_view, i18n("Dialogs\n\n"
																		"Selecting a dialog in the list will open the dialog "
																		"definition file and dispay the dialog for editing."));
																		
	whats_this->add((QWidget*)kdlg_prop_widget, i18n("Properties\n\n"
																		"The properties window shows the properties for the currently "
																		"selected widget item. You can switch to the different items "
																		"by selecting them in the editing view or by using the drop-down "
																		"menu at the top of the properties window containing a list with "
																		"all used widget items."));
																		
	whats_this->add((QWidget*)kdlg_edit_widget, i18n("Widget Editing Window\n\n"
																		"The Widget Editing Window is the working window of the widget editor "
																		"displaying a resizable widget with a grid to place selected widget items "
																		"on. A click on the items will acitvate them and give a resize-frame around "
																		"with draggable points to resize the item. The properties for the selected item "
																		"are shown in the properties window at the right. New widget items can be added "
																		"by choosing them on the widgets-window on the left."));
																		
	whats_this->add(header_widget, i18n("Header/Resource Files\n\n"
	                                  "This window shows header and all other project files "
	                                  "except for source files. The statusbar shows the "
	                                  "status of the insert/overwrite key as well as the "
	                                  "cursor position by line and column."));
	
	whats_this->add(cpp_widget, i18n("C/C++ Files\n\n"
	                                  "This window shows only source files. The statusbar shows "
	                                  "the status of the insert/overwrite key as well as the "
	                                  "cursor position by line and column."));
	
	whats_this->add(browser_widget, i18n("Documentation Browser\n\n"
	                                  "The documentation browser window shows the online-"
	                                  "documentation provided with kdevelop as well as "
	                                  "library class documentation created. Use the documentation "
	                                  "tree to switch between various parts of the documentation."));	
	
	whats_this->add(messages_widget, i18n("Messages\n\n"
	                                  "The messages window shows the output of the compiler and "
	                                  "used utilities like kdoc reference documentation. "
	                                  "For compiler error messages, click on the error message. "
	                                  "This will automatically open the source file and set the "
	                                  "cursor to the line that caused the compiler error/warning. "));

	// this doesn't work because of the ClickFocus policy of the stdin/stdout widget. NoFocus functions but
	// disables the functionality of the widget...:-(                                  	
	/*	whats_this->add(stdin_stdout_widget, i18n("StdOut\n\n"
		"The Standard Input/Standard Output window is a replacement "
		"for terminal-based application communication. Running "
		"terminal applications are using this instead of a terminal window."));
	*/
whats_this->add(stderr_widget, i18n("StdErr\n\n""The Standard Error window displays messages of running applications ""using the cerr function to debug."));
	                                  																												
}
















