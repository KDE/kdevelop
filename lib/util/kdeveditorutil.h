/***************************************************************************
 *   Copyright (C) 2007 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVEDITOR_H
#define KDEVEDITOR_H

namespace KTextEditor
{
class Document;
class View;
}

#include <qstring.h>

/**
Class with some common utility operations not currently supported by KTE

	@author Jens Dagerbo <jens.dagerbo@swipnet.se>
*/
class KDevEditorUtil
{
    KDevEditorUtil() {}
public:

	/**
	 * 
	 * @param line 
	 * @param col 
	 * @param doc 
	 * @param view 
	 * @return 
	 */
	static bool currentPositionReal( unsigned int * line, unsigned int * col, KTextEditor::Document * doc, KTextEditor::View * view = 0 );

	/**
	 * 
	 * @param doc 
	 * @param view 
	 * @return 
	 */
	static QString currentLine( KTextEditor::Document * doc, KTextEditor::View * view = 0 );

	/**
	 * Call to get the text under the cursor of the currently active view.
	 * @return the text under the cursor of the currently active view
	 */
	static QString currentWord( KTextEditor::Document * doc, KTextEditor::View * view = 0 );

	/**
	 *  Call to get the selection in the currently active view
	 * @return the selection in the currently active view
	 */
	static QString currentSelection( KTextEditor::Document * doc );
};

#endif


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs on;
