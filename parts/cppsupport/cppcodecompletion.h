/***************************************************************************
                          cppcodecompletion.h  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CPPCODECOMPLETION_H__
#define __CPPCODECOMPLETION_H__

#include <qobject.h>

#include "keditor/editor.h"
#include "keditor/edit_iface.h"

class CppCodeCompletion : public QObject
{
	Q_OBJECT

	public:
		CppCodeCompletion ( KEditor::Editor* pEditor );
		virtual ~CppCodeCompletion();

	protected slots:
		void slotDocumentActivated ( KEditor::Document* pDoc );
		void slotCursorPositionChanged ( KEditor::Document* pDoc, int nLine, int nCol );

	private:
		KEditor::Editor* m_pEditor;
};

#endif
