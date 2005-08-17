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

#ifndef DATABASE2_H
#define DATABASE2_H

#include "qfeatures.h"
//Added by qt3to4:
#include <QEvent>
#include <Q3SqlForm>

#ifndef QT_NO_SQL
#include <q3dataview.h>
#include <q3databrowser.h>
#include <q3sqlcursor.h>
#include <qstring.h>

class QSqlDatabase;
class Q3SqlForm;

class DatabaseSupport2
{
public:
    DatabaseSupport2();
    virtual ~DatabaseSupport2() {}

    void initPreview( const QString &connection, const QString &table, QObject *o,
		      const QMap<QString, QString> &databaseControls );

protected:
    QSqlDatabase* con;
    Q3SqlForm* frm;
    QString tbl;
    QMap<QString, QString> dbControls;
    QObject *parent;

};

class QDesignerDataBrowser2 : public Q3DataBrowser, public DatabaseSupport2
{
    Q_OBJECT

public:
    QDesignerDataBrowser2( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );
};

class QDesignerDataView2 : public Q3DataView, public DatabaseSupport2
{
    Q_OBJECT

public:
    QDesignerDataView2( QWidget *parent, const char *name );

protected:
    bool event( QEvent* e );

};

#define DatabaseSupport DatabaseSupport2
#define QDesignerDataBrowser QDesignerDataBrowser2
#define QDesignerDataView QDesignerDataView2

#endif

#endif
