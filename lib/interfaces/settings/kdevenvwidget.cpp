/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "kdevenvwidget.h"

#include <QHeaderView>

#include <kdebug.h>

#include "kdevapi.h"
#include "kdevconfig.h"

KDevEnvWidget::KDevEnvWidget( QWidget *parent )
        : QDialog( parent )
{
    setupUi( parent );

    variableTable->setColumnCount( 2 );
    variableTable->setRowCount( 0 );
    variableTable->verticalHeader() ->hide();
    variableTable->horizontalHeader() ->setStretchLastSection ( true );
    connect( variableTable, SIGNAL( cellChanged( int, int ) ),
             this, SLOT( settingsChanged( int, int ) ) );
    connect( variableTable, SIGNAL( currentCellChanged( int, int, int, int ) ),
             this, SLOT( focusChanged( int, int, int, int ) ) );

    connect( newButton, SIGNAL( clicked() ),
             this, SLOT( newButtonClicked() ) );
    connect( deleteButton, SIGNAL( clicked() ),
             this, SLOT( deleteButtonClicked() ) );
    connect( processDefaultButton, SIGNAL( clicked() ),
             this, SLOT( processDefaultButtonClicked() ) );
}

KDevEnvWidget::~KDevEnvWidget()
{}

void KDevEnvWidget::loadSettings()
{
    kDebug() << k_funcinfo << endl;

    variableTable->blockSignals( true );

    //Clear the maps
    m_overrides.clear();
    m_processDefaults.clear();
    m_currentOverrides.clear();
    m_currentProcessDefaults.clear();

    //Clear the table widget
    variableTable->clearContents();
    variableTable->setRowCount( 0 );

    int i = 0;
    EnvironmentMap ovrMap = KDevApi::self() ->environment() ->overrideMap();
    EnvironmentMap::const_iterator it = ovrMap.constBegin();
    for ( ; it != ovrMap.constEnd(); ++it )
    {
        variableTable->insertRow( i );

        QTableWidgetItem * name = new QTableWidgetItem( it.key() );
        setOverride( name );
        variableTable->setItem( i, 0, name );
        name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        QTableWidgetItem * value = new QTableWidgetItem( it.value() );
        setOverride( value );
        variableTable->setItem( i, 1, value );

        //Store this in a map for easy diffs
        m_overrides.insert( it.key(), it.value() );
        i++;
    }

    int i2 = 0;
    EnvironmentMap proMap = KDevApi::self() ->environment() ->processDefaultMap();
    EnvironmentMap::const_iterator it2 = proMap.constBegin();
    for ( ; it2 != proMap.constEnd(); ++it2 )
    {
        if ( KDevApi::self() ->environment() ->isOverride( it2.key() ) )
            continue;

        variableTable->insertRow( i2 );

        QTableWidgetItem * name = new QTableWidgetItem( it2.key() );
        setProcessDefault( name );
        variableTable->setItem( i2, 0, name );
        name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        QTableWidgetItem * value = new QTableWidgetItem( it2.value() );
        setProcessDefault( value );
        variableTable->setItem( i2, 1, value );

        //Store this in a map for easy diffs
        m_processDefaults.insert( it2.key(), it2.value() );
        i2++;
    }

    //FIXME This is a bug in Qt... #123207
    //     variableTable->setSortingEnabled( true );
    //     variableTable->sortItems( 0, Qt::DescendingOrder );
    variableTable->blockSignals( false );
}

void KDevEnvWidget::saveSettings()
{
    kDebug() << k_funcinfo << endl;
    //make sure the maps are set
    generateCurrentMaps();
    KDevApi::self() ->environment() ->saveSettings( m_currentOverrides );
}

void KDevEnvWidget::defaults()
{
    kDebug() << k_funcinfo << endl;
}

void KDevEnvWidget::newButtonClicked()
{
    kDebug() << k_funcinfo << endl;
}

void KDevEnvWidget::deleteButtonClicked()
{
    kDebug() << k_funcinfo << endl;
}

void KDevEnvWidget::processDefaultButtonClicked()
{
    kDebug() << k_funcinfo << endl;
}

void KDevEnvWidget::settingsChanged( int row, int /*column*/ )
{
    QTableWidgetItem * name = variableTable->item( row, 0 );
    QTableWidgetItem * value = variableTable->item( row, 1 );
    QString _name = name->text();
    QString _value = value->text();

    bool o = isOverride( name );
    bool p = isProcessDefault( name );
    QString oValue = KDevApi::self() ->environment() ->override( _name );
    QString pValue = KDevApi::self() ->environment() ->processDefault( _name );

    // 1. Process default is changed to override
    if ( p && _value != pValue )
    {
        setOverride( name );
        setOverride( value );
    }
    // 2. Override is changed back to process default
    else if ( o && _value == pValue )
    {
        setProcessDefault( name );
        setProcessDefault( value );
    }
    emit changed( diff() );
}

void KDevEnvWidget::focusChanged( int row, int /*column*/, int, int )
{
    kDebug() << k_funcinfo << endl;
    QString name = variableTable->item( row, 0 )->text();
    QString value = variableTable->item( row, 1 )->text();

    bool pDefault = KDevApi::self() ->environment()->isProcessDefault( name );
    QString pValue = KDevApi::self() ->environment() ->processDefault( name );

    // Var is not processDefault?
    deleteButton->setEnabled( !pDefault );

    // Var is processDefault and value is changed?
    processDefaultButton->setEnabled( pDefault && pValue != value );
}

bool KDevEnvWidget::isOverride( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::red );
}

bool KDevEnvWidget::isProcessDefault( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::black );
}

void KDevEnvWidget::setOverride( QTableWidgetItem *item )
{
    item->setTextColor( Qt::red );
}

void KDevEnvWidget::setProcessDefault( QTableWidgetItem * item )
{
    item->setTextColor( Qt::black );
}

bool KDevEnvWidget::diff()
{
    generateCurrentMaps();
    return ( m_currentOverrides != m_overrides
             || m_currentProcessDefaults != m_processDefaults );
}

void KDevEnvWidget::generateCurrentMaps()
{
    //create maps of current settings to compare
    m_currentOverrides.clear();
    m_currentProcessDefaults.clear();
    int rows = variableTable->rowCount();
    for ( int row = 0; row < rows; ++row )
    {
        QTableWidgetItem * name = variableTable->item( row, 0 );
        QTableWidgetItem * value = variableTable->item( row, 1 );
        QString _name = name->text();
        QString _value = value->text();
        if ( isOverride( name ) )
            m_currentOverrides.insert( _name, _value );
        else if ( isProcessDefault( name ) )
            m_currentProcessDefaults.insert( _name, _value );
    }
}

#include "kdevenvwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
