 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef EDITORINTERFACE_H
#define EDITORINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qmap.h>
#include <qvaluelist.h>

class QWidget;
class QObjectList;
class QObject;

// {8668161a-6037-4220-86b6-ccaa20127df8}
#ifndef IID_Editor
#define IID_Editor QUuid( 0x8668161a, 0x6037, 0x4220, 0x86, 0xb6, 0xcc, 0xaa, 0x20, 0x12, 0x7d, 0xf8 )
#endif

struct EditorInterface : public QUnknownInterface
{
    enum Mode
    {
	Editing,
	Debugging
    };

    virtual QWidget *editor( bool readOnly,
			     QWidget *parent,
			     QUnknownInterface *designerIface ) = 0;

    virtual void setText( const QString &txt ) = 0;
    virtual QString text() const = 0;
    virtual bool isUndoAvailable() const = 0;
    virtual bool isRedoAvailable() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void cut() = 0;
    virtual void copy() = 0;
    virtual void paste() = 0;
    virtual void selectAll() = 0;
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor ) = 0;
    virtual bool replace( const QString &find, const QString &replace, bool cs, bool wo, bool forward, bool startAtCursor, bool replaceAll ) = 0;
    virtual void gotoLine( int line ) = 0;
    virtual void indent() = 0;
    virtual void scrollTo( const QString &txt, const QString &first ) = 0;
    virtual void splitView() = 0;
    virtual void setContext( QObject *this_ ) = 0;
    virtual void setError( int line ) = 0;
    virtual void setStep( int line ) = 0;
    virtual void setStackFrame( int line ) = 0;
    virtual void clearStep() = 0;
    virtual void clearStackFrame() = 0;
    virtual void readSettings() = 0;
    virtual void setModified( bool m ) = 0;
    virtual bool isModified() const = 0;
    virtual int numLines() const = 0;
    virtual void breakPoints( QValueList<uint> &l ) const = 0;
    virtual void setBreakPoints( const QValueList<uint> &l ) = 0;
    virtual void setMode( Mode m ) = 0;

    virtual void onBreakPointChange( QObject *receiver, const char *slot ) = 0;

};

#endif
