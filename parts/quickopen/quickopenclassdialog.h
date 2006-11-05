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

#include "quickopendialog.h"
#include <codemodel.h>
#include <qstringlist.h>

class QuickOpenPart;

class QuickOpenClassDialog : public QuickOpenDialog
{
    Q_OBJECT
public:
    QuickOpenClassDialog(QuickOpenPart* part, QWidget* parent = 0, const char* name = 0, bool modal = TRUE, WFlags fl = 0 );
    virtual ~QuickOpenClassDialog();

public slots:
    virtual void slotExecuted( QListBoxItem* );
    virtual void slotReturnPressed();

protected:
    void findAllClasses( QStringList& classList );
    void findAllClasses( QStringList& classList, const ClassDom klass );
    void findAllClasses( QStringList& classList, const NamespaceDom ns );

    ClassList findClass( const QString& name );
    ClassList findClass( QStringList& path, const NamespaceDom ns );
    ClassList findClass( QStringList& path, const ClassList klasses );
    ClassList findClass( QStringList& path, const ClassDom klass );

protected slots:
    virtual void accept();

private:
    QStringList m_classList;
};

#endif

