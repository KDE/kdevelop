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

#include <qcombobox.h>
#include <qlineedit.h>

#include <kpassdlg.h>
#include <knuminput.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "serverconfigurationwidget.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

const QString SSS( ":" );  // Server String Separator :)

///////////////////////////////////////////////////////////////////////////////
// class ServerConfigurationWidget
///////////////////////////////////////////////////////////////////////////////

ServerConfigurationWidget::ServerConfigurationWidget( QWidget *parent, const char *name, WFlags f )
    : ServerConfigurationWidgetBase( parent, name ? name : "serverconfigurationwidget", f )
{
    setWFlags( WDestructiveClose | getWFlags() );  // Auto-delete this window when closed

    connect( userNameLineEdit, SIGNAL(lostFocus()), this, SLOT(slotBuildServerString()) );
    connect( serverPathLineEdit, SIGNAL(lostFocus()), this, SLOT(slotBuildServerString()) );
    connect( connectionMethodComboBox, SIGNAL(activated(int)), this, SLOT(slotBuildServerString()) );

    // Update others widgets when connection method changes
    connect( connectionMethodComboBox, SIGNAL(activated(const QString &)), this, SLOT(slotConnectionMethodChanged(const QString &)) );
}

///////////////////////////////////////////////////////////////////////////////

ServerConfigurationWidget::~ServerConfigurationWidget()
{
}

///////////////////////////////////////////////////////////////////////////////

// Will attempt to build a server configuration string: this is called every time
// the user fills a field
void ServerConfigurationWidget::slotBuildServerString()
{
    if (serverPath().isEmpty())
        return;

    QString serverString = QString::null;
    if (connectionMethod() == "local" && !serverPath().isEmpty())
    {
        serverString = serverPath();
    }
    else
    {
        if (userName().isEmpty())
            return;

        QString serverString = SSS + connectionMethod() + SSS +
            userName() + "@" + serverName() + ":" + serverPort() + serverPath();
    }

    KMessageBox::information( 0, serverString, "Server string is ..." );

    emit serverStringReady( serverString );
}

///////////////////////////////////////////////////////////////////////////////

void ServerConfigurationWidget::slotConnectionMethodChanged( const QString &connMethod )
{
    kdDebug( 9999 ) << "ServerConfigurationWidget::slotConnectionMethodChanged( QString &)" << endl;
    kdDebug( 9999 ) << "connMethod == " << connMethod << endl;

    if (connMethod == "local")
    {
        this->userNameLineEdit->setDisabled( true );
        this->serverNameLineEdit->setDisabled( true );
        this->serverPortNumInput->setDisabled( true );
    }
    else // connMethod == "pserver" || connMethod == "ext"
    {
        this->userNameLineEdit->setEnabled( true );
        this->serverNameLineEdit->setEnabled( true );
        this->serverPortNumInput->setEnabled( true );
    }
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::connectionMethod() const
{
    return connectionMethodComboBox->text( connectionMethodComboBox->currentItem() );
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::userName() const
{
    return userNameLineEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QCString ServerConfigurationWidget::password() const
{
//    m_password = QCString( "xxx" );

    (int)KPasswordDialog::getPassword( m_password,
        i18n("Please, type to your account password for accessing the repository\n"
            "(Leave blank if no password is needed)") );
/*
    if (KPasswordDialog::getPassword( m_password,
        i18n("Please, type to your account password for accessing the repository\n"
            "(Leave blank if no password is needed)") ) == KDialog::Accepted)
    {
    }
*/
    return m_password;
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::serverName() const
{
    return serverNameLineEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::serverPort() const
{
    return QString::number( serverPortNumInput->value() );
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::serverPath() const
{
    return serverPathLineEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString ServerConfigurationWidget::compressionLevel() const
{
    return compressionLevelComboBox->text( compressionLevelComboBox->currentItem() );
}

//#include "serverconfigurationwidget.moc.cpp"
