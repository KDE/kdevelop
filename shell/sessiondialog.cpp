/* This file is part of KDevelop
Copyright 2008 Anreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "sessiondialog.h"
#include "ui_sessiondialog.h"

#include <QtCore/QDebug>

#include <kmessagebox.h>

#include "core.h"
#include "sessioncontroller.h"
#include "session.h"

using KDevelop::Core;
using KDevelop::SessionController;
using KDevelop::Session;

QString defaultNewButtonTooltip = "";
QString defaultNewButtonWhatsthis = "";
const QString newSessionName = "New Session";

SessionDialog::SessionDialog( QWidget* parent )
    : KDialog( parent ), m_ui( new Ui::SessionDialog )
{
    setButtons( KDialog::Close );
    setCaption( i18n( "Configure Sessions" ) );
    m_ui->setupUi( mainWidget() );
    SessionController* ctrl = Core::self()->sessionController();
    Q_FOREACH( const QString& name, ctrl->sessions() )
    {
        createAndSetupItem( ctrl->session( name ) );
    }
    defaultNewButtonTooltip = m_ui->newButton->toolTip();
    defaultNewButtonWhatsthis = m_ui->newButton->whatsThis();
    connect( m_ui->newButton, SIGNAL(clicked()), this, SLOT(createSession()) );
    connect( m_ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteSession()) );
    connect( m_ui->sessionList, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT(renameSession(QListWidgetItem*)));
    connect( m_ui->sessionList, SIGNAL( itemSelectionChanged() ), m_ui->deleteButton, SLOT(setEnabled()) );
    enableNewButton();
}

void SessionDialog::createSession()
{
    SessionController* ctrl = Core::self()->sessionController();
    Session* s = ctrl->createSession( "New Session" );
    QListWidgetItem* item = createAndSetupItem( s );
    enableNewButton();
    m_ui->sessionList->editItem( item );
}

void SessionDialog::enableNewButton()
{
    bool enable = m_ui->sessionList->findItems( "New Session", Qt::MatchExactly ).isEmpty();
    m_ui->newButton->setEnabled( enable );
    if( enable )
    {
        m_ui->newButton->setToolTip( defaultNewButtonTooltip );
        m_ui->newButton->setWhatsThis( defaultNewButtonWhatsthis );
    } else {
        m_ui->newButton->setToolTip( i18n("Disabled because there's already a Session named 'New Session' " ) );
        m_ui->newButton->setWhatsThis( i18n("<b>New Session</b><p>This button is disabled because there exists a Session with the"
        "name 'New Session'. Rename or Delete that session to enable this button again") );
    }
}

void SessionDialog::deleteSession()
{
    foreach( QListWidgetItem* item, m_ui->sessionList->selectedItems() )
    {
        Core::self()->sessionController()->deleteSession( item->text() );
        itemSessionMap.remove( item );
        delete item;
    }
    m_ui->deleteButton->setEnabled( false );
    enableNewButton();
}

void SessionDialog::renameSession( QListWidgetItem* item )
{
    Session* s = itemSessionMap.value( item );

    if( item->text() != s->name() )
    {
        s->setName( item->text() );
        enableNewButton();
    }
    if( item->checkState() == Qt::Checked && itemSessionMap.value( item ) != Core::self()->activeSession() )
    {
        for( int i = 0; i < m_ui->sessionList->model()->rowCount(); i++ )
        {
            if( m_ui->sessionList->item( i )->checkState() == Qt::Checked && m_ui->sessionList->item( i ) != item )
            {
                m_ui->sessionList->item( i )->setCheckState( Qt::Unchecked );
            }
        }
        Core::self()->sessionController()->loadSession( item->text() );
    }
}

QListWidgetItem* SessionDialog::createAndSetupItem( Session* session )
{
    QListWidgetItem* item = new QListWidgetItem( session->name(), m_ui->sessionList );
    itemSessionMap.insert( item, session );
    item->setFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    if( session == Core::self()->activeSession() )
    {
        item->setCheckState( Qt::Checked );
    } else
    {
        item->setCheckState( Qt::Unchecked );
    }
    return item;
}
