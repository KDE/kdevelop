/*
 * This file is part of KDevelop
 * Copyright 2012 Przemysław Czekaj <xcojack@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "openwithconfig.h"

#include <KDebug>

OpenWithConfig::OpenWithConfig( const QString& mimeType, QWidget* parent )
: KDialog( parent )
, mimeType( mimeType )
{
    QWidget *widget = new QWidget( this );

    configWidget = new Ui::OpenWithConfigUI;
    configWidget->setupUi( widget );

    setCaption( i18n( "Open With Configuration" ) );

    setButtons( KDialog::Ok );
    enableButton( KDialog::Ok, true );

    setMainWidget( widget );

    connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );

    serviceModel = new QStandardItemModel( this );

    configWidget->services->setModel( serviceModel );

    configWidget->internal->setCheckState( Qt::CheckState::Checked );
    configWidget->services->setDisabled( true );

    connect( configWidget->services, SIGNAL( clicked( QModelIndex ) ), this, SLOT( serviceSelected( QModelIndex ) ) );
    connect( configWidget->internal, SIGNAL( toggled( bool ) ), this, SLOT( internalCheckboxToggle( bool ) ) );
}

OpenWithConfig::~OpenWithConfig()
{
}

void OpenWithConfig::addItem( const KService::Ptr& service )
{
    QStandardItem *item = new QStandardItem( service->name() );
    item->setIcon( SmallIcon( service->icon() ) );

    item->setEditable( false );

    QMap<QString, QVariant> data;
    data.insert( "storageId", QVariant( service->storageId() ) );

    KConfigGroup config = KGlobal::config()->group( "Open With Defaults" );
    QString storageid = config.readEntry( mimeType, QString() );

    item->setData( QVariant( data ) );
    serviceModel->appendRow( item );

    if( service->storageId() == storageid ) {
        configWidget->services->selectionModel()->select( item->index(), QItemSelectionModel::Select );
        configWidget->services->setDisabled( false );
        configWidget->internal->setCheckState( Qt::CheckState::Unchecked );
    }
}

void OpenWithConfig::manageOkButton()
{
    if( configWidget->internal->isChecked() || serviceItem ) {
        enableButton( KDialog::Ok, true );
    } else {
        enableButton( KDialog::Ok, false );
    }
}

void OpenWithConfig::serviceSelected( const QModelIndex& index )
{
    if( index.isValid() ) {
        serviceItem = serviceModel->itemFromIndex( index );
    } else {
        serviceItem = 0;
    }

    manageOkButton();
}

void OpenWithConfig::internalCheckboxToggle( bool checked )
{
    configWidget->services->setDisabled( checked );

    manageOkButton();
}

void OpenWithConfig::save()
{
    KConfigGroup config = KGlobal::config()->group( "Open With Defaults" );
    QString storageid = config.readEntry( mimeType, QString() );

    if( configWidget->internal->isChecked() ) {
        if( !storageid.isEmpty() ) {
            kDebug(9532) << "remove entry";
            config.deleteEntry( mimeType );
        }
    } else if( serviceItem ) {
        QMap<QString, QVariant> data = serviceItem->data().toMap();
        storageid = data.value( "storageId" ).toString();
        config.writeEntry( mimeType, storageid );
    }
}

#include "openwithconfig.moc"
