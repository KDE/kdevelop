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

#ifndef DATABASE_H
#define DATABASE_H

#include "qfeatures.h"

#ifndef QT_NO_SQL
#include <qdataview.h>
#include <qdatabrowser.h>
#include <qsqlcursor.h>
#include <qstring.h>

class QSqlDatabase;
class QSqlForm;

class DatabaseSupport
{
public:
    DatabaseSupport();
    virtual ~DatabaseSupport() {}

    void initPreview( const QString &connection, const QString &table, QObject *o,
		      const QMap<QString, QString> &databaseControls );

protected:
    QSqlDatabase* con;
    QSqlForm* frm;
    QString tbl;
    QMap<QString, QString> dbControls;
    QObject *parent;

};

class QDesignerDataBrowser : public QDataBrowser, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerDataBrowser( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );
};

class QDesignerDataView : public QDataView, public DatabaseSupport
{
    Q_OBJECT

public:
    QDesignerDataView( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );

};
#endif

#endif
