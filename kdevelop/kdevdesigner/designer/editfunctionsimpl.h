/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef EDITFUNCTIONSIMPL_H
#define EDITFUNCTIONSIMPL_H

#include "editfunctions.h"
#include "hierarchyview.h"
#include "metadatabase.h"
#include <qmap.h>

class FormWindow;
class QListViewItem;

class EditFunctions : public EditFunctionsBase
{
    Q_OBJECT

public:
    EditFunctions( QWidget *parent, FormWindow *fw, bool showOnlySlots = FALSE );

    void setCurrentFunction( const QString &function );
    void functionAdd( const QString &access = QString::null,
		      const QString &type = QString::null  );
    void functionAdd() { functionAdd( "public" ); }

signals:
    void itemRenamed(const QString &);

protected slots:
    void okClicked();
    void functionRemove();
    void currentItemChanged( QListViewItem * );
    void currentTextChanged( const QString &txt );
    void currentSpecifierChanged( const QString &s );
    void currentAccessChanged( const QString &a );
    void currentReturnTypeChanged( const QString &type );
    void currentTypeChanged( const QString &type );
    void displaySlots( bool justSlots );
    void emitItemRenamed( QListViewItem *, int, const QString & );

private:
    enum Attribute { Name, Specifier, Access, ReturnType, Type };
    struct FunctItem {
	int id;
	QString oldName;
	QString newName;
	QString oldRetTyp;
	QString retTyp;
	QString spec;
	QString oldSpec;
	QString access;
	QString oldAccess;
	QString type;
	QString oldType;

	Q_DUMMY_COMPARISON_OPERATOR( FunctItem )
    };

    void changeItem( QListViewItem *item, Attribute a, const QString &nV );

    FormWindow *formWindow;
    QMap<QListViewItem*, int> functionIds;
    QStringList removedFunctions;
    QValueList<MetaDataBase::Function> itemList;
    QValueList<FunctItem> functList;
    int id;
    QString lastType;
};

#endif
