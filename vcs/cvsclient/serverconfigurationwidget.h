/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SERVERCONFIGURATIONWIDGET_H
#define SERVERCONFIGURATIONWIDGET_H

#include <serverconfigurationwidgetbase.h>

/**
* Encapsulates user input while configuring access to remote CVS repository
* and provide some useful signal and methods to possible parent widgets.
*
* @author Mario Scalas
*/
class ServerConfigurationWidget : public ServerConfigurationWidgetBase
{
    Q_OBJECT
public:
    ServerConfigurationWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~ServerConfigurationWidget();

    QString connectionMethod() const;
    QString userName() const;
    QCString password() const;
    QString serverName() const;
    QString serverPort() const;
    QString serverPath() const;
    QString compressionLevel() const;

signals:
    void serverStringReady( const QString &serverString );

private slots:
    void slotBuildServerString();
    void slotConnectionMethodChanged( const QString &connMethod );

private:
    mutable QCString m_password;
};

#endif
