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

#include "kdeveditorutil.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>

bool KDevEditorUtil::currentPositionReal( unsigned int * line, unsigned int * col, KTextEditor::Document * doc, KTextEditor::View * view )
{
	if ( !line || !col ) return false;

	KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
	if ( !editIface ) return false;

	view = view ? view : dynamic_cast<KTextEditor::View*>( doc->widget() );

	KTextEditor::ViewCursorInterface * cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( view );
	if ( !cursorIface ) return false;

	cursorIface->cursorPositionReal( line, col );
	return true;
}

QString KDevEditorUtil::currentLine( KTextEditor::Document * doc, KTextEditor::View * view )
{
	KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
	if ( !editIface ) return QString();

	view = view ? view : dynamic_cast<KTextEditor::View*>( doc->widget() );

	KTextEditor::ViewCursorInterface * cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( view );
	if ( !cursorIface ) return QString();

	uint line = 0;
	uint col = 0;
	cursorIface->cursorPositionReal(&line, &col);

	return editIface->textLine(line);
}

QString KDevEditorUtil::currentWord( KTextEditor::Document * doc, KTextEditor::View * view )
{
	KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
	if ( !editIface ) return QString();

	view = view ? view : dynamic_cast<KTextEditor::View*>( doc->widget() );

	KTextEditor::ViewCursorInterface * cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( view );
	if ( !cursorIface ) return QString();

	uint line = 0;
	uint col = 0;
	cursorIface->cursorPositionReal(&line, &col);

	QString linestr = editIface->textLine(line);

	int startPos = QMAX( QMIN( (int)col, (int)linestr.length()-1 ), 0 );
	int endPos = startPos;
    startPos--;
	while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' || linestr[startPos] == '~') )
		startPos--;
	while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
		endPos++;

	return ( ( startPos == endPos ) ? QString::null : linestr.mid( startPos+1, endPos-startPos-1 ) );
}


QString KDevEditorUtil::currentSelection( KTextEditor::Document * doc )
{
	KTextEditor::SelectionInterface * selectIface = dynamic_cast<KTextEditor::SelectionInterface*>( doc );
	return selectIface ? selectIface->selection() : QString();
}


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs on;

