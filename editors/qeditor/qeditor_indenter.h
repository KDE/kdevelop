/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


/*
 *  Copyright (C) 2001 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef QEDITOR_INDENTER_H
#define QEDITOR_INDENTER_H

#include <qstring.h>
#include <qvariant.h>
#include <qmap.h>
#include <private/qrichtext_p.h>

class QEditorPart;
class KDialogBase;
class QEditor;

class QEditorIndenter: public QTextIndent{
public:
    QEditorIndenter( QEditor* );
    virtual ~QEditorIndenter();

    virtual QWidget* createConfigPage( QEditorPart*, KDialogBase* =0, const char* =0 ) { return 0; }

    virtual const QMap<QString, QVariant>& values() const { return m_values; }
    virtual void updateValues( const QMap<QString, QVariant>& values ) { m_values = values; }

    QEditor* editor() const { return m_editor; }
    
    virtual void tabify( QString& text );
    virtual void indentLine( QTextParagraph *p, int &oldIndent, int &newIndent );
    virtual int indentation( const QString& s );

    virtual int previousNonBlankLine( int line );
    virtual int indentForLine( int line ) = 0;
    virtual void indent( QTextDocument*, QTextParagraph*, int* =0, int* =0 );

protected:
    QEditor* m_editor;
    QMap<QString, QVariant> m_values;
};


#endif
