/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#ifndef SOURCEEDITOR_H
#define SOURCEEDITOR_H

#include "../interfaces/editorinterface.h"
#include <qvbox.h>
#include <qguardedptr.h>

class FormWindow;
class QCloseEvent;
struct LanguageInterface;
class Project;
class SourceFile;

class SourceEditor : public QVBox
{
    Q_OBJECT

public:
    SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface );
    ~SourceEditor();

    void setObject( QObject *fw, Project *p );
    QObject *object() const { return obj; }
    Project *project() const { return pro; }
    void setFunction( const QString &func, const QString &clss = QString::null );
    void setClass( const QString &clss );
    void save();
    bool saveAs();
    void setModified( bool b );
    bool isModified() const;

    static QString sourceOfObject( QObject *fw, const QString &lang, EditorInterface *iface, LanguageInterface *lIface );

    QString language() const;
    void setLanguage( const QString &l );

    void editCut();
    void editCopy();
    void editPaste();
    bool editIsUndoAvailable();
    bool editIsRedoAvailable();
    void editUndo();
    void editRedo();
    void editSelectAll();

    void configChanged();
    void refresh( bool allowSave );
    void resetContext();

    EditorInterface *editorInterface() const { return iFace; }

    void setFocus();
    int numLines() const;
    void saveBreakPoints();
    void clearStep();
    void clearStackFrame();
    void resetBreakPoints();

    QString text() const;

    void checkTimeStamp();

    SourceFile *sourceFile() const;
    FormWindow *formWindow() const;

protected:
    void closeEvent( QCloseEvent *e );


private:
    EditorInterface *iFace;
    LanguageInterface *lIface;
    QGuardedPtr<QObject> obj;
    Project *pro;
    QString lang;
    QGuardedPtr<QWidget> editor;

};

#endif
