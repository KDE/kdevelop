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
// #include <stringsextracttoolview.h>
// #include <stringsextracttool.h>
#include <infotoolview.h>
#include <infotool.h>
#include <filtertoolview.h>
#include <filtertool.h>
// #include <checksumtoolview.h>
// #include <checksumtool.h>
#include <documentinfotoolview.h>
#include <documentinfotool.h>
// #include <poddecodertoolview.h>
// #include <poddecodertool.h>
#include <bytetabletoolview.h>
#include <bytetabletool.h>
#include <bookmarkstoolview.h>
#include <bookmarkstool.h>
// #include <structtoolview.h>
// #include <structtool.h>
// KDev
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
// KDE
#include <KAboutData>
#include <kaction.h>
#include <kmimetype.h>
#include <kxmlguiwindow.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kstandardaction.h>
#include <kicon.h>
#include <kactioncollection.h>
// Qt
#include <QObject>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QPluginLoader>


K_PLUGIN_FACTORY(OktetaPluginFactory, registerPlugin<KDevelop::OktetaPlugin>(); )
K_EXPORT_PLUGIN( OktetaPluginFactory( KAboutData( "kdevokteta","kdevokteta", ki18n("Okteta"), "0.1", ki18n("A Hex Editor"), KAboutData::License_GPL)))

namespace KDevelop
{

OktetaPlugin::OktetaPlugin( QObject* parent, const QVariantList& args )
  : IPlugin( OktetaPluginFactory::componentData(), parent ),
    mDocumentFactory( new OktetaDocumentFactory(this) )
{
    Q_UNUSED(args)

//     addTool( new ChecksumToolView(new ChecksumTool()), "ChecksumToolView" );
    addTool( new Kasten::FilterToolView(new Kasten::FilterTool()), "FilterToolView" );
//     addTool( new StringsExtractToolView(new StringsExtractTool()), "StringsExtractToolView" );
    addTool( new Kasten::ByteTableToolView(new Kasten::ByteTableTool()), "ByteTableToolView" );
    addTool( new Kasten::InfoToolView(new Kasten::InfoTool()), "InfoToolView" );
//     addTool( new PODDecoderToolView(new PODDecoderTool()), "PODDecoderToolView" );
//     addTool( new StructToolView(new StructTool()), "StructToolView" );
    addTool( new Kasten::BookmarksToolView(new Kasten::BookmarksTool()), "BookmarksToolView" );

    KDevelop::IDocumentController* idc = core()->documentController();
    idc->registerDocumentForMimetype("audio/x-wav", mDocumentFactory);
}


// bool OktetaPlugin::saveFile()
// {
//     KSaveFile uiFile( localFilePath() );
//     //FIXME: find a way to return an error. KSaveFile
//     //doesn't use the KIO error codes
//     if ( !uiFile.open() )
//         return false;
//
//     QTextStream stream ( &uiFile );
//     QByteArray windowXml = m_window->contents().toUtf8();
//     stream << windowXml;
//
//     if ( !uiFile.finalize() )
//         return false;
//
//     m_window->setDirty(false);
//     setModified(false);
//     return true;
// }
/*
void OktetaPlugin::saveActiveDocument()
{
    kDebug(9038) << "going to save:" << mActiveDocument;
    if( mActiveDocument )
    {
        mActiveDocument->save( KDevelop::IDocument::Default );
    }
}

void OktetaPlugin::activateDocument( KDevelop::IDocument* doc )
{
    if( doc->mimeType()->is( "application/x-designer" ) )
    {
        kDebug(9038) << "Doc activated:" << doc;
        mActiveDocument = doc;
    }
}*/

void OktetaPlugin::addTool( Kasten::AbstractToolView* toolView, const QString& id )
{
//     if( dockWidget->isVisible() && mCurrentView )
//         toolView->tool()->setTargetModel( mCurrentView );

//     connect( dockWidget, SIGNAL(visibilityChanged( bool )), SLOT(onToolVisibilityChanged( bool )) );
    OktetaToolViewFactory* factory = new OktetaToolViewFactory( toolView, id );

    core()->uiController()->addToolView( toolView->title(), factory );
}

OktetaPlugin::~OktetaPlugin()
{
    delete mDocumentFactory;
}

}
