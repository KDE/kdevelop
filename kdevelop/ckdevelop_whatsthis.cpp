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

 
#include <ckdevelop.h>
 
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
																													
	whats_this->add(toolBar()->getButton(ID_EDIT_UNDO),i18n("Undo\n\n"
	                                                        "Reverts the last editing step."));
	whats_this->add(toolBar()->getButton(ID_EDIT_REDO),i18n("Redo\n\n"
	                                                        "If an editing step was undone, redo "
	                                                        "lets you do this step again."));
																																																											
	whats_this->add(class_tree, i18n("Class viewer\n\nThe class viewer shows the project(s), classes,"
																	 "classmembers and variables found by scanning the"
																		"header files in the source tree."
																		"Clicking on the class name results in opening the "
																		"header file,setting the cursor to the class "
																		"declaration line."
																		"Clicking on the member function results in opening "
																		"the source file and sets the cursor to the member "
																		"implementation. Selecting a member variable switches "
																		"to the variable declaration in the header file.\n"
																		"You can enable/diable the treeview window by selecting "
																		"Tree-View in the View-menu or use the key accelerator."));
																														
}
