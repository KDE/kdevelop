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

#ifndef MULTILINEEDITORIMPL_H
#define MULTILINEEDITORIMPL_H

#include <qaction.h>
#include <qtextedit.h>
#include <private/qrichtext_p.h>
#include "multilineeditor.h"

class FormWindow;
class QToolBar;
class QTextDocument;

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, const char *name = 0 );
    QTextDocument *document() const { return QTextEdit::document(); }
    QTextParagraph *paragraph();

};

class ToolBarItem : public QAction
{
    Q_OBJECT

public:
    ToolBarItem( QWidget *parent, QWidget *toolBar,
                 const QString &label, const QString &tagstr,
		 const QIconSet &icon, const QKeySequence &key = 0 );
    ~ToolBarItem();
signals:
    void clicked( const QString &t );

protected slots:
    void wasActivated();
private:
    QString tag;
};


class MultiLineEditor : public MultiLineEditorBase
{
    Q_OBJECT

public:
    MultiLineEditor( bool call_static, bool richtextMode, QWidget *parent, QWidget *editWidget,
		     FormWindow *fw, const QString &text = QString::null );
    bool useWrapping() const;
    void setUseWrapping( bool );
    static QString getText( QWidget *parent, const QString &text, bool richtextMode, bool *useWrap );
    int exec();

protected slots:
    void okClicked();
    void applyClicked();
    void cancelClicked();
    void insertTags( const QString& );
    void insertBR();
    void showFontDialog();
    void changeWrapMode( bool );

    void closeEvent( QCloseEvent *e );

private:
    QString getStaticText();
    TextEdit *textEdit;
    QTextEdit *mlined;
    FormWindow *formwindow;
    QToolBar *basicToolBar;
    QToolBar *fontToolBar;
    QToolBar *optionsToolBar;
    QAction *wrapAction;
    int res;
    QString staticText;
    bool callStatic;
    bool oldDoWrap, doWrap;
    QVariant oldWrapMode;
    QString oldWrapString;
};

#endif
