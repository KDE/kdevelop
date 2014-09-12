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

#include "oktetawidget.h"

// plugin
#include "oktetadocument.h"
#include "oktetaplugin.h"
// Okteta Kasten
#include <bytearrayview.h>
#include <bytearrayrawfilesynchronizerfactory.h>
#include <overwriteonlycontroller.h>
#include <overwritemodecontroller.h>
#include <gotooffsetcontroller.h>
#include <selectrangecontroller.h>
#include <searchcontroller.h>
#include <replacecontroller.h>
#include <bookmarkscontroller.h>
#include <printcontroller.h>
#include <viewconfigcontroller.h>
#include <viewmodecontroller.h>
#include <viewstatuscontroller.h>
#if KASTEN_VERSION == 2
#include <viewprofilecontroller.h>
#include <viewprofilesmanagecontroller.h>
#endif
// Kasten
#include <readonlycontroller.h>
// #include <document/readonly/readonlybarcontroller.h>
// #include <io/synchronize/synchronizecontroller.h>
#include <clipboardcontroller.h>
#include <insertcontroller.h>
#include <copyascontroller.h>
#include <exportcontroller.h>
#include <versioncontroller.h>
#include <zoomcontroller.h>
#include <zoombarcontroller.h>
#include <selectcontroller.h>
// KDevelop
#include <sublime/view.h>
// KDE
#include <KLocalizedString>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
// Qt
#include <QVBoxLayout>

#include <kdebug.h>

namespace KDevelop
{

OktetaWidget::OktetaWidget( QWidget* parent, Kasten::ByteArrayView* byteArrayView, OktetaPlugin* plugin )
  : QWidget( parent ),
    KXMLGUIClient(),
    mByteArrayView( byteArrayView )
{
    setComponentData( plugin->componentData() );
    setXMLFile( "kdevokteta.rc" );

    setupActions(plugin);

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    QWidget* widget = mByteArrayView->widget();
    layout->addWidget( widget );
    setFocusProxy( widget );
}

void OktetaWidget::setupActions(OktetaPlugin* plugin)
{
    mControllers.append( new Kasten::VersionController(this) );
    mControllers.append( new Kasten::ReadOnlyController(this) );
    // TODO: save_as
//     mControllers.append( new ExportController(mProgram->viewManager(),mProgram->documentManager(),this) );
    mControllers.append( new Kasten::ZoomController(this) );
    mControllers.append( new Kasten::SelectController(this) );
    mControllers.append( new Kasten::ClipboardController(this) );
//     if( modus != BrowserViewModus )
//         mControllers.append( new Kasten::InsertController(mProgram->viewManager(),mProgram->documentManager(),this) );
//     mControllers.append( new Kasten::CopyAsController(mProgram->viewManager(),mProgram->documentManager(),this) );
    mControllers.append( new Kasten::OverwriteModeController(this) );
    mControllers.append( new Kasten::SearchController(this,this) );
    mControllers.append( new Kasten::ReplaceController(this,this) );
//     mControllers.append( new Kasten::GotoOffsetController(mGroupedViews,this) );
//     mControllers.append( new Kasten::SelectRangeController(mGroupedViews,this) );
    mControllers.append( new Kasten::BookmarksController(this) );
    mControllers.append( new Kasten::PrintController( this ) );
    mControllers.append( new Kasten::ViewConfigController(this) );
    mControllers.append( new Kasten::ViewModeController(this) );
#if KASTEN_VERSION == 2
    Kasten::ByteArrayViewProfileManager* viewProfileManager = plugin->viewProfileManager();
    mControllers.append( new Kasten::ViewProfileController(viewProfileManager, mByteArrayView->widget(), this) );
    mControllers.append( new Kasten::ViewProfilesManageController(this, viewProfileManager, mByteArrayView->widget()) );
    // update the text of the viewprofiles_manage action, to make clear this is just for byte arrays
    QAction* viewprofilesManageAction = actionCollection()->action(QLatin1String("settings_viewprofiles_manage"));
    viewprofilesManageAction->setText( i18nc("@action:inmenu",
                                             "Manage Byte Array View Profiles...") );
#endif

//     Kasten::StatusBar* bottomBar = static_cast<Kasten::StatusBar*>( statusBar() );
//     mControllers.append( new ViewStatusController(bottomBar) );
//     mControllers.append( new ReadOnlyBarController(bottomBar) );
//     mControllers.append( new ZoomBarController(bottomBar) );

    foreach( Kasten::AbstractXmlGuiController* controller, mControllers )
        controller->setTargetModel( mByteArrayView );
#if 0
    QDesignerFormWindowManagerInterface* manager = mDocument->form()->core()->formWindowManager();
    KActionCollection* ac = actionCollection();

    KStandardAction::save( this, SLOT(save()), ac);
    ac->addAction( "adjust_size", manager->actionAdjustSize() );
    ac->addAction( "break_layout", manager->actionBreakLayout() );
    ac->addAction( "designer_cut", manager->actionCut() );
    ac->addAction( "designer_copy", manager->actionCopy() );
    ac->addAction( "designer_paste", manager->actionPaste() );
    ac->addAction( "designer_delete", manager->actionDelete() );
    ac->addAction( "layout_grid", manager->actionGridLayout() );
    ac->addAction( "layout_horiz", manager->actionHorizontalLayout() );
    ac->addAction( "layout_vertical", manager->actionVerticalLayout() );
    ac->addAction( "layout_split_horiz", manager->actionSplitHorizontal() );
    ac->addAction( "layout_split_vert", manager->actionSplitVertical() );
    ac->addAction( "designer_undo", manager->actionUndo() );
    ac->addAction( "designer_redo", manager->actionRedo() );
    ac->addAction( "designer_select_all", manager->actionSelectAll() );
#endif
}
#if 0
void OktetaWidget::save()
{
    mDocument->save();
}
#endif

OktetaWidget::~OktetaWidget()
{
    qDeleteAll( mControllers );
}

}
