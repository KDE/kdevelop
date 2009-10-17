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
#include <kdiroperator.h>
#include <kactioncollection.h>

namespace KDevelop
{

OpenProjectPage::OpenProjectPage( const KUrl& startUrl, QWidget* parent )
        : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    KUrl start = Core::self()->projectController()->projectsBaseDirectory();
    if(startUrl.isValid())
        start = startUrl;
    
    fileWidget = new KFileWidget( start, this);

    QStringList filters;
    QStringList allEntry;
    allEntry << "*."+ShellExtension::getInstance()->projectFileExtension();
    filters << QString( "%1|%2 (%1)").arg("*."+ShellExtension::getInstance()->projectFileExtension()).arg(ShellExtension::getInstance()->projectFileDescription());
    foreach(const KPluginInfo& info, PluginController::queryExtensionPlugins( "org.kdevelop.IProjectFileManager" ) )
    {
        QVariant filter = info.property("X-KDevelop-ProjectFilesFilter");
	    QVariant desc = info.property("X-KDevelop-ProjectFilesFilterDescription");
        QString filterline;
        if( filter.isValid() && desc.isValid() )
        {
            m_projectFilters.insert( info.name(), filter.toStringList() );
            allEntry += filter.toStringList();
            filters << QString("%1|%2 (%1)").arg(filter.toStringList().join(" ")).arg(desc.toString());
        }
    }
	
    filters.prepend( QString( "%1|All Project Files (%1)").arg(allEntry.join(" ") ) );

    fileWidget->setFilter( filters.join("\n") );

    fileWidget->setMode( KFile::Modes( KFile::File | KFile::Directory | KFile::ExistingOnly ) );

    KConfigGroup projectdialogsettings = KGlobal::config()->group( "Open Project Dialog Settings" );
#if KDE_IS_VERSION(4,3,60)
    fileWidget->readConfig( projectdialogsettings );
#else
    projectdialogsettings.writeEntry( "View Style", "Simple" );
    // Enforce "short view" for now as KFileWidget has no API to read its config from a different
    // but the global-default for KFileDialog.
    // TODO for 4.4: Add the needed API to KFileWidget
    fileWidget->dirOperator()->readConfig( projectdialogsettings );
    fileWidget->dirOperator()->setViewConfig( projectdialogsettings );
#endif

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
    emit urlSelected( getAbsoluteUrl( file ) );
}

void OpenProjectPage::opsEntered( const KUrl& url )
{
    emit urlSelected( getAbsoluteUrl( url.url() ) );
}

void OpenProjectPage::comboTextChanged( const QString& file )
{
    emit urlSelected( getAbsoluteUrl( file ) );
}

QMap<QString,QStringList> OpenProjectPage::projectFilters() const
{
    return m_projectFilters;
}

}

#include "openprojectpage.moc"
