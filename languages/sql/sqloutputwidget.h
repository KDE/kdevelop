/***************************************************************************
 *   Copyright (C) 2003 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SQLOUTPUTWIDGET_H_
#define _SQLOUTPUTWIDGET_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QSqlError>

class Q3WidgetStack;
class Q3DataTable;
class Q3TextEdit;
class QSqlError;

class SqlOutputWidget : public QWidget
{
    Q_OBJECT

public:
    SqlOutputWidget( QWidget* parent = 0, const char* name = 0 );
    virtual ~SqlOutputWidget();

public:
    void showQuery( const QString& connectionName, const QString& query );
    void showError( const QSqlError& error );

private:
    void showError( const QString& msg );
    void showSuccess( int rowsAffected );

    Q3WidgetStack* m_stack;
    Q3DataTable* m_table;
    Q3TextEdit* m_textEdit;
};

#endif
