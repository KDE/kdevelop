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

#include <kconfiggroup.h>
#include <kurlcompletion.h>
#include <kurlcombobox.h>
#include <kurlpixmapprovider.h>
#include <kglobal.h>
#include <kfileitem.h>
#include <ksharedconfig.h>
#include <kdebug.h>
#include <kplugininfo.h>
#include <kfilewidget.h>

#include "shellextension.h"
#include "core.h"
#include "projectcontroller.h"
#include "plugincontroller.h"

namespace KDevelop
{

OpenProjectPage::OpenProjectPage( QWidget* parent )
        : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    fileWidget = new KFileWidget( Core::self()->projectController()->projectsBaseDirectory(), this);

    QStringList filters;
    QStringList allEntry;
    allEntry << "*."+ShellExtension::getInstance()->projectFileExtension();
    filters << "*."+ShellExtension::getInstance()->projectFileExtension() +"|"+ShellExtension::getInstance()->projectFileDescription();
    KPluginInfo::List plugins = PluginController::queryExtensionPlugins( "org.kdevelop.IProjectFileManager" );
    kDebug() << "checking plugins for files filter:" << plugins.size();
    foreach(const KPluginInfo& info, PluginController::queryExtensionPlugins( "org.kdevelop.IProjectFileManager" ) )
    {
        kDebug() << "Checking plugin:" << info.name();
        QVariant filter = info.property("X-KDevelop-ProjectFilesFilter");
        QVariant desc = info.property("X-KDevelop-ProjectFilesFilterDescription");
        kDebug() << "filter:" << filter << "desc:" << desc;
        QString filterline;
        if( filter.isValid() && desc.isValid() )
        {
            allEntry.append( filter.toString() );
            filters << filter.toStringList().join(" ")+"|"+desc.toString();
        }
    }

    filters.prepend( allEntry.join(" ")+"|All Project Files" );

    fileWidget->setFilter( filters.join("\n") );

    fileWidget->setMode( KFile::Modes( KFile::File | KFile::Directory | KFile::ExistingOnly ) );
    layout->addWidget( fileWidget );

    QWidget* ops= fileWidget->findChild<QWidget*>( "KFileWidget::ops" );
    // Emitted for changes in the places view, the url navigator and when using the back/forward/up buttons
    connect( ops, SIGNAL(urlEntered(const KUrl&)), SLOT(opsEntered(const KUrl&)));

    // Emitted when selecting an entry from the "Name" box or editing in there
    connect( fileWidget->locationEdit(), SIGNAL(editTextChanged(const QString&)), 
             SLOT(comboTextChanged(const QString&)));

    // Emitted when clicking on a file in the fileview area
    connect( fileWidget, SIGNAL(fileHighlighted(const QString&)), SLOT(highlightFile(const QString&)) );
}

KUrl OpenProjectPage::getAbsoluteUrl( const QString& file ) const
{
    KUrl u(file);
    if( u.isRelative() )
    {
        u = fileWidget->baseUrl();
        u.addPath( file );
    }
    return u;
}

void OpenProjectPage::highlightFile( const QString& file )
{
    kDebug() << "highlightfile:" << file << fileWidget->selectedUrl();
    emit urlSelected( getAbsoluteUrl( file ) );
}

void OpenProjectPage::opsEntered( const KUrl& url )
{
    kDebug() << "entered:" << url << fileWidget->selectedUrl();
    emit urlSelected( getAbsoluteUrl( url.url() ) );
}

void OpenProjectPage::comboTextChanged( const QString& file )
{
    kDebug() << "Text changed:" << file << fileWidget->selectedUrl();
    emit urlSelected( getAbsoluteUrl( file ) );
}

}

#include "openprojectpage.moc"
