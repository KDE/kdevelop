/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef QUICKOPENCLASSDIALOG_H
#define QUICKOPENCLASSDIALOG_H

#include "quickopenbase.h"
#include <codemodel.h>
#include <qstringlist.h>

class QuickOpenPart;
class KCompletion;

class QuickOpenClassDialog : public QuickOpenDialogBase
{
    Q_OBJECT
public:
    QuickOpenClassDialog(QuickOpenPart* part, QWidget* parent = 0, const char* name = 0, bool modal = TRUE, WFlags fl = 0 );
    virtual ~QuickOpenClassDialog();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void slotExecuted( QListBoxItem* );
    virtual void slotReturnPressed();
    virtual void slotTextChanged( const QString& text );

protected:
    /*$PROTECTED_FUNCTIONS$*/
    void findAllClasses( QStringList& classList );
    void findAllClasses( QStringList& classList, const ClassDom klass );
    void findAllClasses( QStringList& classList, const NamespaceDom ns );

    ClassDom findClass( const QString& name );
    ClassDom findClass( QStringList& path, const NamespaceDom ns );
    ClassDom findClass( QStringList& path, const ClassDom klass );

protected slots:
    /*$PROTECTED_SLOTS$*/
    virtual void reject();
    virtual void accept();

    void moveUpInList();
    void moveDownInList();
    void scrollUpInList();
    void scrollDownInList();
    void goToBegin();
    void goToEnd();

private:
    QuickOpenPart* m_part;
    KCompletion* m_completion;
    QStringList m_classList;
};

#endif

