/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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

#ifndef QCOMPLETIONEDIT_H
#define QCOMPLETIONEDIT_H

#include <qlineedit.h>
#include <qstringlist.h>

class QListBox;
class QVBox;

class QCompletionEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( bool autoAdd READ autoAdd WRITE setAutoAdd )
    Q_PROPERTY( bool caseSensitive READ isCaseSensitive WRITE setCaseSensitive )

public:
    QCompletionEdit( QWidget *parent = 0, const char *name = 0 );

    bool autoAdd() const;
    QStringList completionList() const;
    bool eventFilter( QObject *o, QEvent *e );
    bool isCaseSensitive() const;

public slots:
    void setCompletionList( const QStringList &l );
    void setAutoAdd( bool add );
    void clear();
    void addCompletionEntry( const QString &entry );
    void removeCompletionEntry( const QString &entry );
    void setCaseSensitive( bool b );

signals:
    void chosen( const QString &text );

private slots:
    void textDidChange( const QString &text );

private:
    void placeListBox();
    void updateListBox();

private:
    bool aAdd;
    QStringList compList;
    QListBox *listbox;
    QVBox *popup;
    bool caseSensitive;

};



#endif
