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
#include "cdocbrowser.h"
 
void CKDevelop::initWhatsThis(){
	whats_this->add(toolBar()->getButton(ID_PROJECT_OPEN), i18n("Open project\n\n"
																															"Shows the open project dialog "
																															"to select a project to be opened"));
																																
	whats_this->add(toolBar()->getButton(ID_FILE_OPEN),i18n("Open file\n\n"
																													"Shows the open file dialog to "
																													 "select a file to be opened."));																												
	
	whats_this->add(toolBar()->getButton(ID_FILE_SAVE),i18n("Save file\n\n"
																													"Saves the file opened in the actual "
																													"editing view"));
	
	whats_this->add(toolBar()->getButton(ID_FILE_PRINT),i18n("Print\n\n"
	                                                        "Opens the printing dialog. There, you can "
	                                                        "configure which printing program you wish "
	                                                        "to use, either a2ps or ensrcipt, and print "
	                                                        "your project files."));
	
	whats_this->add(toolBar()->getButton(ID_EDIT_UNDO),i18n("Undo\n\n"
	                                                        "Reverts the last editing step."));
	
	whats_this->add(toolBar()->getButton(ID_EDIT_REDO),i18n("Redo\n\n"
	                                                        "If an editing step was undone, redo "
	                                                        "lets you do this step again."));

	whats_this->add(toolBar()->getButton(ID_EDIT_CUT),i18n("Cut\n\n"
	                                                        "Cuts out the selected text and copies "
	                                                        "it to the system clipboard."));	                                                        	
	
	whats_this->add(toolBar()->getButton(ID_EDIT_COPY),i18n("Copy\n\n"
	                                                        "Copies the selected text into the "
	                                                        "system clipboard."));
	
	whats_this->add(toolBar()->getButton(ID_EDIT_PASTE),i18n("Paste\n\n"
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
	
	whats_this->add(toolBar()->getButton(ID_BUILD_MAKE),i18n("Make\n\n"
	                                                          "Invokes the make-command set in the "
	                                                          "options-menu for the current project "
	                                                          "after saving all files. "
	                                                          "This will compile all changed sources "
	                                                          "since the last compilation was invoked.\n"
	                                                          "The output window opens to show compiler "
	                                                          "messages. If errors occur, clicking on the "
	                                                          "error line will open the file where the "
	                                                          "error was found and sets the cursor to the "
	                                                          "error line."));
	
	whats_this->add(toolBar()->getButton(ID_BUILD_REBUILD_ALL),i18n("Rebuild all\n\n"
	                                                                "After saving all files, rebuild all "
	                                                                "invokes the make-command set with the "
	                                                                "clean-option to remove all object files. "
	                                                                "Then, configure creates new Makefiles and "
	                                                                "the make-command will rebuild the project."));
	
	whats_this->add(toolBar()->getButton(ID_BUILD_RUN),i18n("Execute\n\n"
	                                                        "After saving all files,the make-command is "
	                                                        "called to build the project. Then the binary "
	                                                        "is executed out of the project directory.\n"
	                                                        "Be aware that this function is only valid for "
	                                                        "programs and that references to e.g. pixmaps "
	                                                        "or html help files that are supposed to be "
	                                                        "installed will cause some strange behavoir "
	                                                        "like testing the helpmenu will open an error "
	                                                        "message that the index.html file is not found."));
	
	whats_this->add(toolBar()->getButton(ID_BUILD_DEBUG),i18n("Debug program\n\n"
	                                                          "Opens KDbg in the tools window with your actual"
	                                                          "program and allows you to execute it step by step"
	                                                          "by setting breakpoints in the sourcecode."
	                                                          "Note that you have to open your sourcecodes in KDbg"
	                                                          "to access it."));
	
	whats_this->add(toolBar()->getButton(ID_BUILD_STOP),i18n("Stop\n\n"
	                                                          "If activated, the stop-command will interrupt "
	                                                          "the active process. This affects make-commands "
	                                                          "as well as documentation generation."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(TOOLBAR_CLASS_CHOICE),i18n("Classes\n\n"
	                                                    "Choosing a class will switch to the header "
	                                                    "file where the class is declarated."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(TOOLBAR_METHOD_CHOICE),i18n("Methods\n\n"
	                                                    "Allows selecting a method of the current "
	                                                    "selected class. This will switch to the "
	                                                    "implementation of the chosen class."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_DOC_BACK),i18n("Documentation-Back\n\n"
	                                                    "This opens the previous browser page in the "
	                                                    "browser window. "));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_DOC_FORWARD),i18n("Documentation-Forward\n\n"
	                                                    "This opens the next opened browser page after "
	                                                    "Documentation-Back was chosen."));
	
	whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getButton(ID_DOC_SEARCH_TEXT),i18n("Search Marked Text\n\n"
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
	/*	whats_this->add(stdin_stdout_widget, i18n("StdIn/StdOut\n\n"
		"The Standard Input/Standard Output window is a replacement "
		"for terminal-based application communication. Running "
		"terminal applications are using this instead of a terminal window."));
	*/
	                                  	
	whats_this->add(stderr_widget, i18n("StdErr\n\n"
	                                    "The Standard Error window displays messages of running applications "
	                                    "using the cerr function to debug."));
	                                  																												
}



