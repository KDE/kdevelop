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

#include "sqlactions.h"

#include <q3popupmenu.h>
#include <qstringlist.h>
#include <qsqldatabase.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kcombobox.h>

#include "kdevplugin.h"
#include "kdevlanguagesupport.h"
#include "sqlsupport_part.h"

SqlListAction::SqlListAction(SQLSupportPart *part, const QString &text, 
				 const KShortcut& cut,
                                 const QObject *receiver, const char *slot,
                                 KActionCollection *parent, const char *name)
    : KWidgetAction( m_combo = new KComboBox(), text, cut, 0, 0, parent, name), m_part(part)
{
#if (QT_VERSION >= 0x030100)
    m_combo->setEditable( false );
    m_combo->setAutoCompletion( true );
#endif

    m_combo->setMinimumWidth( 200 );
    m_combo->setMaximumWidth( 400 );

    connect( m_combo, SIGNAL(activated(const QString&)), receiver, slot );
    connect( m_combo, SIGNAL(activated(int)), this, SLOT(activated(int)) );

    setShortcutConfigurable( false );
    setAutoSized( true );

    refresh();
}


void SqlListAction::setCurrentConnectionName(const QString &name)
{
    int idx = m_part->connections().findIndex( name );
    if ( idx < 0 )
        m_combo->setCurrentItem( 0 );
    else
        m_combo->setCurrentItem( idx + 1 );
}


QString SqlListAction::currentConnectionName() const
{
    if ( m_combo->currentItem() <= 0 )
        return QString::null;
    return m_part->connections()[ m_combo->currentItem() - 1 ];
}

void SqlListAction::activated(int idx)
{
    if (idx < 1 || (int)m_part->connections().count() <= idx)
        return;
    const QSqlDatabase *db = QSqlDatabase::database(m_part->connections()[idx], true);
    m_combo->changeItem( db->isOpen() ? SmallIcon( "ok" ) : SmallIcon( "no" ), 
                         m_combo->text(idx), idx );
}

void SqlListAction::refresh()
{
    const QStringList& dbc = m_part->connections();

    m_combo->clear();
    m_combo->insertItem( i18n("<no database server>") );

    QString cName;
    for ( QStringList::ConstIterator it = dbc.begin(); it != dbc.end(); ++it ) {

        QSqlDatabase* db = QSqlDatabase::database( (*it), false );
        if ( !db ) {
            kdDebug( 9000 ) << "Could not find database connection " << (*it) << endl;
            m_combo->insertItem( SmallIcon( "no" ), i18n("<error - no connection %1>").arg( *it ) );
            continue;
        }
        cName = db->driverName();
        cName.append( "://" ).append( db->userName() ).append( "@" ).append( db->hostName() );
        cName.append( "/" ).append( db->databaseName() );

        m_combo->insertItem( db->open() ? SmallIcon( "ok" ) : SmallIcon( "no" ), cName );
    }
}


#include "sqlactions.moc"
