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
#include "ceditwidget.h"
#include "clogfileview.h"
#include "coutputwidget.h"
#include "crealfileview.h"
#include "doctreeview.h"

#include <kcombobox.h>
#include <khtmlview.h>
#include <klocale.h>
#include <ktoolbarbutton.h>

#include <qwhatsthis.h>

void CKDevelop::initWhatsThis(){
  whats_this->add(toolBar()->getButton(ID_PROJECT_OPEN), i18n("Open project\n\n"
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

  QString build_rebuild_all_str=i18n("Rebuild all\n\n"
        "After saving all files, rebuild all "
        "invokes the make-command set with the "
        "clean-option to remove all object files. "
        "Then, configure creates new Makefiles and "
        "the make-command will rebuild the project.");
  whats_this->add(toolBar()->getButton(ID_BUILD_REBUILD_ALL),build_rebuild_all_str);

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

  QString build_debug_str=i18n("Debug program\n\n"
        "Runs your program in the debugger you have chosen "
        "(by default the internal debugger is used) "
         "All debuggers should allow you to execute your program "
        "step by step by setting breakpoints in the sourcecode." );

  whats_this->add(toolBar()->getButton(ID_DEBUG_START),build_debug_str);

  QString debug_run_str=i18n("Continue application execution\n\n"
        "Continues the execution of your application in the "
        "debugger. This only has affect when the application "
        "has been halted by the debugger (ie. a breakpoint has been "
        "activated or the interrupt was pressed).");
  whats_this->add(toolBar()->getButton(ID_DEBUG_RUN), debug_run_str);

  QString debug_step_str=i18n("Step into\n\n"
        "Executes exactly one line of source. If the source line is "
        "a call to a function then execution will stop after "
        "the function has been entered ");
  whats_this->add(toolBar()->getButton(ID_DEBUG_STEP), debug_step_str);

  QString debug_next_str=i18n("Step over\n\n"
        "Executes one line of source in the current source file. "
        "If the source line is a call to a function the whole function "
        "is executed and the app will stop at the line following the "
        "function call.");
  whats_this->add(toolBar()->getButton(ID_DEBUG_NEXT), debug_next_str);

  QString debug_finish_str=i18n("Step out of\n\n"
        "Executes the application until the currently executing "
        "function is completed. The debugger will then display the "
        "line after the original call to that function. If we are in "
        "the outermost frame (ie in main()), then this operation "
        "has no affect.");
  whats_this->add(toolBar()->getButton(ID_DEBUG_FINISH), debug_finish_str);

  QString build_stop_str=i18n("Stop\n\n"
        "If activated, the stop-command will interrupt "
        "the active process. This affects make-commands "
        "as well as documentation generation.");
  whats_this->add(toolBar()->getButton(ID_BUILD_STOP),build_stop_str);


  QString output=i18n("Output-View\n\n"
        "Enables/Disables the output window. The "
        "state of the window is displayed by the "
        "toggle button: if the button is pressed, "
        "the window is open, otherwise closed.");
                      
  whats_this->add(toolBar()->getButton(ID_VIEW_OUTPUTVIEW),output);

  QString treeview=i18n("Tree-View\n\n"
        "Enables/Disables the tree window. The "
        "state of the window is displayed by the "
        "toggle button: if the button is pressed, "
        "the window is open, otherwise closed.");
  whats_this->add(toolBar()->getButton(ID_VIEW_TREEVIEW),treeview);

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
  
  whats_this->add(browser_widget->view(), i18n("Documentation Browser\n\n"
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
  /*  whats_this->add(stdin_stdout_widget, i18n("StdOut\n\n"
    "The Standard Input/Standard Output window is a replacement "
    "for terminal-based application communication. Running "
    "terminal applications are using this instead of a terminal window."));
  */
  whats_this->add(stderr_widget, i18n("StdErr\n\n""The Standard Error window displays messages of running applications ""using the cerr function to debug."));

  if (var_viewer)
    whats_this->add((QWidget*)var_viewer, i18n("Variable Tree\n\n"
        "The variable tree allows you to see "
        "the variable values as you step "
        "through your program using the internal "
        "debugger. Click the RMB on items in "
        "this view to get a popup menu.\n"
        "To speed up stepping through your code "
        "leave the tree items closed and add the "
        "variable(s) to the watch section.\n"
        "To change a variable value in your running app "
        "use a watch variable (eg a=5)."));
  
  if (brkptManager)
    whats_this->add((QWidget*)brkptManager, i18n("Breakpoint list\n\n"
        "Displays a list of breakpoints with "
        "their current status. Clicking on a "
        "breakpoint item with the RMB displays "
        "a popupmenu so you may manipulate the "
        "breakpoint. Double clicking will take you "
        "to the source in the editor window."));
                                                                                            
  if (frameStack)
    whats_this->add((QWidget*)frameStack, i18n("Frame stack display\n\n"
        "Often referred to as the \"call stack\", "
        "this is a list showing what function "
        "is currently active and who called "
        "each function to get to this point in your "
        "program. By clicking on an item "
        "you can see the values in any of the "
        "previous calling functions "
        "See the VAR tab for these frame "
        "variables."));

  if (disassemble)
    whats_this->add((QWidget*)disassemble, i18n("Machine code display\n\n"
        "A machine code view into your running "
        "executable with the current instruction "
        "highlighted. You can step instruction by "
        "instruction using the debuggers toolbar "
        "buttons of \"step over\" instruction and "
        "\"step into\" instruction."));
}
