/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectfileselectionpage.h"
#include "ui_projectfileselectionpage.h"

#include <QtGui/QStringListModel>

namespace KDevelop
{

ProjectFileSelectionPage::ProjectFileSelectionPage( QWidget* parent )
    : QWidget( parent )
{
    ui = new Ui::ProjectFileSelectionPage;
    ui->setupUi( this );
    model = new QStringListModel();
    ui->listView->setModel( model );
    connect( ui->listView->selectionModel(), SIGNAL(currentChanged( const QModelIndex&, const QModelIndex& ) ),
             SLOT( currentChanged( const QModelIndex& ) ) );
}

void ProjectFileSelectionPage::setEntries( const QStringList& entries )
{
    model->setStringList( entries );
}

void ProjectFileSelectionPage::currentChanged( const QModelIndex& idx )
{
    emit fileSelected( idx.data().toString() );
}

}

#include "projectfileselectionpage.moc"

