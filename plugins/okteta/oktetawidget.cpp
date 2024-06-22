/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "oktetawidget.h"

// plugin
#include "oktetadocument.h"
#include "oktetaplugin.h"
// Okteta Kasten
#include <Kasten/Okteta/ByteArrayView>
// Okteta Kasten controllers
#include <Kasten/Okteta/OverwriteModeControllerFactory>
#include <Kasten/Okteta/GotoOffsetControllerFactory>
#include <Kasten/Okteta/SelectRangeControllerFactory>
#include <Kasten/Okteta/SearchControllerFactory>
#include <Kasten/Okteta/ReplaceControllerFactory>
#include <Kasten/Okteta/BookmarksControllerFactory>
#include <Kasten/Okteta/PrintControllerFactory>
#include <Kasten/Okteta/ViewConfigControllerFactory>
#include <Kasten/Okteta/ViewModeControllerFactory>
#include <Kasten/Okteta/ViewStatusControllerFactory>
#include <Kasten/Okteta/ViewProfileControllerFactory>
#include <Kasten/Okteta/ViewProfilesManageControllerFactory>
// Kasten controllers
#include <Kasten/ReadOnlyControllerFactory>
#include <Kasten/ClipboardControllerFactory>
#include <Kasten/InsertControllerFactory>
#include <Kasten/CopyAsControllerFactory>
#include <Kasten/ExportControllerFactory>
#include <Kasten/VersionControllerFactory>
#include <Kasten/ZoomControllerFactory>
#include <Kasten/ZoomBarControllerFactory>
#include <Kasten/SelectControllerFactory>
// Kasten
#include <Kasten/AbstractXmlGuiController>
// KDevelop
#include <sublime/view.h>
// KF
#include <KLocalizedString>
#include <KStandardAction>
#include <KActionCollection>
// Qt
#include <QVBoxLayout>
#include <QAction>


namespace KDevelop
{

OktetaWidget::OktetaWidget( QWidget* parent, Kasten::ByteArrayView* byteArrayView, OktetaPlugin* plugin )
  : QWidget( parent ),
    KXMLGUIClient(),
    mByteArrayView( byteArrayView )
{
    setComponentName( QStringLiteral("kdevokteta") , QStringLiteral("KDevelop Okteta"));
    setXMLFile( QStringLiteral("kdevokteta.rc") );

    setupActions(plugin);

    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget* widget = mByteArrayView->widget();
    layout->addWidget( widget );
    setFocusProxy( widget );
}

void OktetaWidget::setupActions(OktetaPlugin* plugin)
{
    Kasten::ByteArrayViewProfileManager* viewProfileManager = plugin->viewProfileManager();
    mControllers = {
        Kasten::VersionControllerFactory().create(this),
        Kasten::ReadOnlyControllerFactory().create(this),
        Kasten::ZoomControllerFactory().create(this),
        Kasten::SelectControllerFactory().create(this),
        Kasten::ClipboardControllerFactory().create(this),
        Kasten::OverwriteModeControllerFactory().create(this),
        Kasten::SearchControllerFactory(this).create(this),
        Kasten::ReplaceControllerFactory(this).create(this),
        Kasten::BookmarksControllerFactory().create(this),
        Kasten::PrintControllerFactory().create(this),
        Kasten::ViewConfigControllerFactory().create(this),
        Kasten::ViewModeControllerFactory().create(this),
        Kasten::ViewProfileControllerFactory(viewProfileManager, mByteArrayView->widget()).create(this),
        Kasten::ViewProfilesManageControllerFactory(viewProfileManager, mByteArrayView->widget()).create(this),
    };
    // update the text of the viewprofiles_manage action, to make clear this is just for byte arrays
    QAction* viewprofilesManageAction = actionCollection()->action(QStringLiteral("settings_viewprofiles_manage"));
    viewprofilesManageAction->setText( i18nc("@action:inmenu",
                                             "Manage Byte Array View Profiles...") );

    for (Kasten::AbstractXmlGuiController* controller : std::as_const(mControllers)) {
        controller->setTargetModel( mByteArrayView );
    }
}

OktetaWidget::~OktetaWidget()
{
    qDeleteAll( mControllers );
}

}

#include "moc_oktetawidget.cpp"
