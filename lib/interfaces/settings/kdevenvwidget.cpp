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

#include "kdevapi.h"
#include "kdevenv.h"
#include <kdebug.h>

#include "kdevconfig.h"

#include <QHeaderView>

KDevEnvWidget::KDevEnvWidget( QWidget *parent )
        : QDialog( parent )
{
    setupUi( parent );

    variableTable->setColumnCount( 2 );

    int i = 0;
    QMap<QString, QString> ovrMap = KDevApi::self() ->environment() ->overrideMap();
    QMap<QString, QString>::const_iterator it = ovrMap.constBegin();
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

        //Store this in a mergeMap for easy diffs
        m_mergeMap.insert( it.key(), it.value() );
        i++;
    }

    int i2 = 0;
    QMap<QString, QString> proMap = KDevApi::self() ->environment() ->processDefaultMap();
    QMap<QString, QString>::const_iterator it2 = proMap.constBegin();
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

        //Store this in a mergeMap for easy diffs
        m_mergeMap.insert( it2.key(), it2.value() );
        i2++;
    }

    variableTable->verticalHeader() ->hide();
    variableTable->horizontalHeader() ->setStretchLastSection ( true );

    connect( variableTable, SIGNAL( cellChanged( int, int ) ),
             this, SLOT( settingsChanged( int, int ) ) );
}

KDevEnvWidget::~KDevEnvWidget()
{}

void KDevEnvWidget::saveChanges()
{}

void KDevEnvWidget::on_new_button_clicked()
{}

void KDevEnvWidget::on_edit_button_clicked()
{}

void KDevEnvWidget::on_delete_button_clicked()
{}

void KDevEnvWidget::settingsChanged( int row, int column )
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

bool KDevEnvWidget::diff() const
{
    //create a merge map of current settings to compare
    QMap<QString, QString> currentMap;

    int rows = variableTable->rowCount();
    for ( int row = 0; row < rows; ++row )
    {
        QTableWidgetItem * name = variableTable->item( row, 0 );
        QTableWidgetItem * value = variableTable->item( row, 1 );
        QString _name = name->text();
        QString _value = value->text();
        currentMap.insert( _name, _value );
    }

    return ( currentMap != m_mergeMap );
}

#include "kdevenvwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
