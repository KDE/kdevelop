/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "oktetaplugin.h"

// plugin
#include "oktetadocumentfactory.h"
#include "oktetatoolviewfactory.h"
#include "oktetadocument.h"
// Okteta Kasten tools
#include <stringsextracttoolviewfactory.h>
#include <stringsextracttoolfactory.h>
#include <infotoolviewfactory.h>
#include <infotoolfactory.h>
#include <filtertoolviewfactory.h>
#include <filtertoolfactory.h>
#include <checksumtoolviewfactory.h>
#include <checksumtoolfactory.h>
// #include <documentinfotoolview.h>
// #include <documentinfotool.h>
#include <poddecodertoolviewfactory.h>
#include <poddecodertoolfactory.h>
#include <bytetabletoolviewfactory.h>
#include <bytetabletoolfactory.h>
#include <bookmarkstoolviewfactory.h>
#include <bookmarkstoolfactory.h>
#include <structurestoolviewfactory.h>
#include <structurestoolfactory.h>
// KDev
#include <project/projectmodel.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
// KDE
#include <KAction>
#include <KAboutData>
#include <KPluginFactory>


K_PLUGIN_FACTORY(OktetaPluginFactory, registerPlugin<KDevelop::OktetaPlugin>(); )
K_EXPORT_PLUGIN( OktetaPluginFactory( KAboutData( "kdevokteta","kdevokteta", ki18n("Okteta Plugin for KDevelop"), "0.1", ki18n("Provides simple Hex Editing"), KAboutData::License_GPL).setProgramIconName("okteta")))

namespace KDevelop
{

static inline
void addTool( IUiController* uiController,
              Kasten::AbstractToolViewFactory* toolViewFactory,
              Kasten::AbstractToolFactory* toolFactory )
{
    OktetaToolViewFactory* factory =
        new OktetaToolViewFactory( toolViewFactory, toolFactory );

    uiController->addToolView( toolViewFactory->title(), factory );
}


OktetaPlugin::OktetaPlugin( QObject* parent, const QVariantList& args )
  : IPlugin( OktetaPluginFactory::componentData(), parent ),
    mDocumentFactory( new OktetaDocumentFactory(this) )
{
    Q_UNUSED(args)

    KLocale* globalLocale = KGlobal::locale();
    globalLocale->insertCatalog( QString::fromLatin1("liboktetacore") );
    globalLocale->insertCatalog( QString::fromLatin1("libkasten") );
    globalLocale->insertCatalog( QString::fromLatin1("liboktetakasten") );

    IUiController* uiController = core()->uiController();
    addTool( uiController, new Kasten::ChecksumToolViewFactory(), new Kasten::ChecksumToolFactory() );
    addTool( uiController, new Kasten::FilterToolViewFactory(), new Kasten::FilterToolFactory() );
    addTool( uiController, new Kasten::StringsExtractToolViewFactory, new Kasten::StringsExtractToolFactory() );
    addTool( uiController, new Kasten::ByteTableToolViewFactory(), new Kasten::ByteTableToolFactory() );
    addTool( uiController, new Kasten::InfoToolViewFactory(), new Kasten::InfoToolFactory() );
    addTool( uiController, new Kasten::PodDecoderToolViewFactory(), new Kasten::PodDecoderToolFactory() );
    addTool( uiController, new Kasten::StructuresToolViewFactory(), new Kasten::StructuresToolFactory() );
    addTool( uiController, new Kasten::BookmarksToolViewFactory, new Kasten::BookmarksToolFactory() );

    KDevelop::IDocumentController* documentController = core()->documentController();
    documentController->registerDocumentForMimetype("application/octet-stream", mDocumentFactory);
}

ContextMenuExtension OktetaPlugin::contextMenuExtension( Context* context )
{
    mContextUrls.clear();

    FileContext* fileContext = dynamic_cast<FileContext*>( context );
    ProjectItemContext* projectItemContext = dynamic_cast<ProjectItemContext*>( context );
    if( fileContext && fileContext->urls().count() > 0 ) {
        KMimeType::Ptr mimetype = KMimeType::findByUrl( fileContext->urls().first() );
        if(!mimetype->is("inode/directory")){
            mContextUrls << fileContext->urls().first();
        }
    }
    else if( projectItemContext ) 
    {
        foreach( ProjectBaseItem* item, projectItemContext->items() )
        {
            ProjectFileItem* file = item->file();
            if( file )
                mContextUrls << file->url();
        }
    }

    if( ! mContextUrls.isEmpty() )
    {
        KAction* openAction = new KAction( i18n("Open As Byte Array"), this );
        openAction->setIcon( KIcon("document-open") );
        connect( openAction, SIGNAL(triggered()), SLOT(onOpenTriggered()) );

        KDevelop::ContextMenuExtension contextMenuExtension;
        contextMenuExtension.addAction( KDevelop::ContextMenuExtension::FileGroup, openAction );
        return contextMenuExtension;
    }

    return KDevelop::IPlugin::contextMenuExtension( context );
}

void OktetaPlugin::onOpenTriggered()
{
    KDevelop::ICore* core = KDevelop::ICore::self();
    IDocumentController* documentController = core->documentController();

    foreach( const KUrl& url, mContextUrls )
    {
        IDocument* existingDocument = documentController->documentForUrl(url);
        if( existingDocument )
            if( ! existingDocument->close() )
                continue;

        IDocument* createdDocument = mDocumentFactory->create( url, core );
        if( createdDocument )
            documentController->openDocument( createdDocument );
    }
}

OktetaPlugin::~OktetaPlugin()
{
    delete mDocumentFactory;
}

}
