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
#include "clogfileview.h"
#include "coutputwidget.h"
#include "crealfileview.h"
#include "doctreeview.h"

#include <kcombobox.h>
#include <khtmlview.h>
#include <klocale.h>
#include <ktoolbarbutton.h>

#include <qwhatsthis.h>

void CKDevelop::initWhatsThis() {

  whats_this->add(toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE),
  			i18n("Compile configurations\n\n"
				"Compile configurations allow different build processes "
				"of the same project. A useful example would be "
				"setting up a configuration for a release (final) version "
				"and one for a debug version of a program, or different "
				"configurations for cross-compiling a project.\n\n"
				"The configuration (Default) can only be used standalone, "
				"as the default build process means the build directory "
				"is the same as the source directory. Other configurations "
				"need a separate build directory.\n\n"
				"To use customized compile configurations, switch to the "
				"(Default) configuration first and run Project->DistClean, "
				"as the source directory must be in the distclean state to use "
				"other directories as the build directory.\n\n"
				"To customize KDevelop for using crosscompilers, set up your "
				"crosscompiling environment in the KDevelop Setup dialog, section "
				"Compiler. To add/customize configurations, open the Project Options "
				"dialog, section Compiler-Settings."));
				
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
    whats_this->add((QWidget*)frameStack, i18n("Call stack display\n\n"
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
