/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "openprojectpage.h"

#include <QtCore/QDir>
#include <QtCore/QModelIndex>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>

#include <kfiletreeview.h>
#include <khistorycombobox.h>
#include <kfileplacesview.h>
#include <kfileplacesmodel.h>
#include <kconfiggroup.h>
#include <kurlcompletion.h>
#include <kurlpixmapprovider.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kdebug.h>

#include "shellextension.h"


namespace KDevelop
{

OpenProjectPage::OpenProjectPage( QWidget* parent )
        : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group( "General Options" );
    QString dir = group.readEntry( "DefaultProjectsDirectory",
                                     QDir::homePath() );

    filePlacesView = new KFilePlacesView( this );
    filePlacesView->setObjectName( "places" );
    filePlacesView->setModel( new KFilePlacesModel( filePlacesView ) );
    layout->addWidget( filePlacesView, 2 );

    QVBoxLayout* vlay = new QVBoxLayout( this );
    layout->addLayout( vlay, 5 );

    fileTreeView = new KFileTreeView( this );
    fileTreeView->setObjectName( "treeview" );
    fileTreeView->setDirOnlyMode( true );
    fileTreeView->header()->setResizeMode(0, QHeaderView::ResizeToContents );
    for (int i = 1; i < fileTreeView->model()->columnCount(); ++i)
    {
        fileTreeView->hideColumn(i);
    }
    vlay->addWidget( fileTreeView, 1 );

    historyCombo = new KHistoryComboBox( this );
    historyCombo->setObjectName( "combo" );
    historyCombo->setLayoutDirection( Qt::LeftToRight );
    historyCombo->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLength );
    historyCombo->setTrapReturnKey( true );
    historyCombo->setPixmapProvider( new KUrlPixmapProvider() );
    KUrlCompletion* comp = new KUrlCompletion();
    comp->setMode( KUrlCompletion::DirCompletion );
    comp->setIgnoreCase( true );
    historyCombo->setCompletionObject( comp, true );
    historyCombo->setAutoDeleteCompletionObject( true );
    historyCombo->setDuplicatesEnabled( false );
    vlay->addWidget( historyCombo, 0 );

    activateUrl( dir );

    // Setup connections
    connect( filePlacesView, SIGNAL( urlChanged( const KUrl& ) ), SLOT( changeUrl( const KUrl& ) ) );

    connect( fileTreeView, SIGNAL( activated( const QModelIndex& ) ), SLOT( expandTreeView( const QModelIndex& ) ) );
    connect( fileTreeView, SIGNAL( currentChanged( const KUrl& ) ), SLOT( changeUrl( const KUrl& ) ) );

    connect( historyCombo, SIGNAL( editTextChanged( const QString& ) ), SLOT( changeUrl( const QString& ) ) );
    connect( historyCombo, SIGNAL( activated( const QString& ) ), SLOT( activateUrl( const QString& ) ) );
    connect( historyCombo, SIGNAL( returnPressed( const QString& ) ), SLOT( activateUrl( const QString& ) ) );

}

void OpenProjectPage::expandTreeView( const QModelIndex& idx )
{
    fileTreeView->setExpanded( idx, !fileTreeView->isExpanded( idx ) );
}

void OpenProjectPage::changeUrl( const KUrl& url )
{
    kDebug() << "changing url to" << url << sender();
    bool blocked = filePlacesView->blockSignals( true );
    filePlacesView->setUrl( url );
    filePlacesView->blockSignals( blocked );
    if( fileTreeView->currentUrl() != url )
    {
        blocked = fileTreeView->blockSignals( true );
        fileTreeView->setCurrentUrl( url );
        fileTreeView->blockSignals( blocked );
    }
    blocked = historyCombo->blockSignals( true );
    if( url.isLocalFile() && historyCombo->currentText() != url.path() )
    {
       historyCombo->setEditText( url.path() );
    } else if( !url.isLocalFile() && historyCombo->currentText() != url.prettyUrl() )
    {
       historyCombo->setEditText( url.prettyUrl() );
    } 
    historyCombo->blockSignals( blocked );
    emit urlSelected( url );
}

void OpenProjectPage::changeUrl( const QString& url )
{
    changeUrl( KUrl( url ) );
}

void OpenProjectPage::activateUrl( const QString& url )
{
    changeUrl( url );
    historyCombo->addToHistory( KUrl(url).prettyUrl() );
}

}

#include "openprojectpage.moc"
