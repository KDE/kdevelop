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
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
// KDE
#include <KAboutData>
#include <kpluginfactory.h>


K_PLUGIN_FACTORY(OktetaPluginFactory, registerPlugin<KDevelop::OktetaPlugin>(); )
K_EXPORT_PLUGIN( OktetaPluginFactory( KAboutData( "kdevokteta","kdevokteta", ki18n("Okteta"), "0.1", ki18n("A Hex Editor"), KAboutData::License_GPL)))

namespace KDevelop
{

OktetaPlugin::OktetaPlugin( QObject* parent, const QVariantList& args )
  : IPlugin( OktetaPluginFactory::componentData(), parent ),
    mDocumentFactory( new OktetaDocumentFactory(this) )
{
    Q_UNUSED(args)

    addTool( new Kasten::ChecksumToolViewFactory(), new Kasten::ChecksumToolFactory(),
             "ChecksumToolView", "accessories-calculator", i18n("Checksum"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::FilterToolViewFactory(), new Kasten::FilterToolFactory(),
             "FilterToolView", "okteta", i18n("Binary Filter"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::StringsExtractToolViewFactory, new Kasten::StringsExtractToolFactory(),
             "StringsExtractToolView", "okteta", i18n("Strings"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::ByteTableToolViewFactory(), new Kasten::ByteTableToolFactory(),
             "ByteTableToolView", "table", i18n("Byte Table"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::InfoToolViewFactory(), new Kasten::InfoToolFactory(),
             "InfoToolView", "okteta", i18n("Statistics"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::PodDecoderToolViewFactory(), new Kasten::PodDecoderToolFactory(),
             "PODDecoderToolView", "okteta", i18n("Decoding Table"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::StructuresToolViewFactory(), new Kasten::StructuresToolFactory(),
             "StructToolView", "okteta", i18n("Structures"),
             Qt::RightDockWidgetArea );
    addTool( new Kasten::BookmarksToolViewFactory, new Kasten::BookmarksToolFactory(),
             "BookmarksToolView", "bookmarks", i18n("Bookmarks"),
             Qt::RightDockWidgetArea );

    KDevelop::IDocumentController* idc = core()->documentController();
    idc->registerDocumentForMimetype("audio/x-wav", mDocumentFactory);
}


void OktetaPlugin::addTool( Kasten::AbstractToolViewFactory* toolViewFactory,
                            Kasten::AbstractToolFactory* toolFactory,
                            const QString& id, const QString& iconName, const QString& title,
                            Qt::DockWidgetArea defaultPosition )
{
//     if( dockWidget->isVisible() && mCurrentView )
//         toolView->tool()->setTargetModel( mCurrentView );

//     connect( dockWidget, SIGNAL(visibilityChanged( bool )), SLOT(onToolVisibilityChanged( bool )) );
    OktetaToolViewFactory* factory =
        new OktetaToolViewFactory( toolViewFactory, toolFactory, iconName, id, defaultPosition );

    core()->uiController()->addToolView( title, factory );
}

OktetaPlugin::~OktetaPlugin()
{
    delete mDocumentFactory;
}

}
