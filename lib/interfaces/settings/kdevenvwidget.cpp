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

    QMap<QString, QString> varMap = KDevApi::self() ->environment() ->variableMap();

    kfcg_variableTable->setColumnCount( 2 );
    kfcg_variableTable->setRowCount( varMap.count() );

    int i = 0;
    QMap<QString, QString>::const_iterator it = varMap.constBegin();
    for ( ; it != varMap.constEnd(); ++it )
    {
        QTableWidgetItem * name = new QTableWidgetItem( it.key() );
        kfcg_variableTable->setItem( i, 0, name );

        QTableWidgetItem * value = new QTableWidgetItem( it.value() );
        kfcg_variableTable->setItem( i, 1, value );
        i++;
    }
    kfcg_variableTable->verticalHeader() ->hide();
    kfcg_variableTable->horizontalHeader() ->setStretchLastSection ( true );

    connect( kfcg_variableTable, SIGNAL( cellChanged( int, int ) ),
             this, SLOT( settingsChanged() ) );
}

KDevEnvWidget::~KDevEnvWidget()
{}

void KDevEnvWidget::on_new_button_clicked()
{}

void KDevEnvWidget::on_edit_button_clicked()
{}

void KDevEnvWidget::on_delete_button_clicked()
{}

void KDevEnvWidget::settingsChanged()
{
    emit changed( true );
}

#include "kdevenvwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
