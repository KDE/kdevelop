/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "mainwindow.h"

#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>

#include <stdlib.h>
#include <qaction.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qsignalmapper.h>
#include <qstylefactory.h>
#include <qworkspace.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qlistbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qinputdialog.h>
#include <qdatetimeedit.h>
#include <qtextedit.h>

#include "defs.h"
#include "project.h"
#include "widgetdatabase.h"
#include "widgetfactory.h"
#include "preferences.h"
#include "formwindow.h"
#include "newformimpl.h"
#include "resource.h"
#include "projectsettingsimpl.h"
#include "workspace.h"
#include "createtemplate.h"
#include "hierarchyview.h"
#include "editfunctionsimpl.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "gotolinedialog.h"
#include "formsettingsimpl.h"
#include "pixmapcollectioneditor.h"
#include "styledbutton.h"
#include "customwidgeteditorimpl.h"
#ifndef QT_NO_SQL
#include "dbconnectionsimpl.h"
#include "dbconnectionimpl.h"
#endif
#include "widgetaction.h"
#include <qtoolbox.h>
#include "startdialogimpl.h"
#include "designerappiface.h"
#include "connectiondialog.h"
#include "configtoolboxdialog.h"
#include "designeraction.h"

#include "kdevdesigner_part.h"

static const char * whatsthis_image[] = {
    "16 16 3 1",
    "	c None",
    "o	c #000000",
    "a	c #000080",
    "o        aaaaa  ",
    "oo      aaa aaa ",
    "ooo    aaa   aaa",
    "oooo   aa     aa",
    "ooooo  aa     aa",
    "oooooo  a    aaa",
    "ooooooo     aaa ",
    "oooooooo   aaa  ",
    "ooooooooo aaa   ",
    "ooooo     aaa   ",
    "oo ooo          ",
    "o  ooo    aaa   ",
    "    ooo   aaa   ",
    "    ooo         ",
    "     ooo        ",
    "     ooo        "};

const QString toolbarHelp = "<p>Toolbars contain a number of buttons to "
"provide quick access to often used functions.%1"
"<br>Click on the toolbar handle to hide the toolbar, "
"or drag and place the toolbar to a different location.</p>";

static QIconSet createIconSet( const QString &name )
{
    QIconSet ic( BarIcon( "" + name, KDevDesignerPartFactory::instance() ) );
    QString prefix = "designer_";
    int right = name.length() - prefix.length();
    ic.setPixmap( BarIcon( prefix + "d_" + name.right( right ), KDevDesignerPartFactory::instance() ),
		  QIconSet::Small, QIconSet::Disabled );
    return ic;
}

void MainWindow::setupEditActions()
{
    actionEditUndo = new DesignerAction( i18n("Undo"), createIconSet( "designer_undo.png" ),i18n("&Undo: Not Available"), CTRL + Key_Z, this, 0 );
    actionEditUndo->setStatusTip( i18n( "Undoes the last action" ) );
    actionEditUndo->setWhatsThis( whatsThisFrom( "Edit|Undo" ) );
    connect( actionEditUndo, SIGNAL( activated() ), this, SLOT( editUndo() ) );
    actionEditUndo->setEnabled( FALSE );

    actionEditRedo = new DesignerAction( i18n( "Redo" ), createIconSet("designer_redo.png"), i18n( "&Redo: Not Available" ), CTRL + Key_Y, this, 0 );
    actionEditRedo->setStatusTip( i18n( "Redoes the last undone operation") );
    actionEditRedo->setWhatsThis( whatsThisFrom( "Edit|Redo" ) );
    connect( actionEditRedo, SIGNAL( activated() ), this, SLOT( editRedo() ) );
    actionEditRedo->setEnabled( FALSE );

    actionEditCut = new DesignerAction( i18n( "Cut" ), createIconSet("designer_editcut.png"), i18n( "Cu&t" ), CTRL + Key_X, this, 0 );
    actionEditCut->setStatusTip( i18n( "Cuts the selected widgets and puts them on the clipboard" ) );
    actionEditCut->setWhatsThis(  whatsThisFrom( "Edit|Cut" ) );
    connect( actionEditCut, SIGNAL( activated() ), this, SLOT( editCut() ) );
    actionEditCut->setEnabled( FALSE );

    actionEditCopy = new DesignerAction( i18n( "Copy" ), createIconSet("designer_editcopy.png"), i18n( "&Copy" ), CTRL + Key_C, this, 0 );
    actionEditCopy->setStatusTip( i18n( "Copies the selected widgets to the clipboard" ) );
    actionEditCopy->setWhatsThis(  whatsThisFrom( "Edit|Copy" ) );
    connect( actionEditCopy, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    actionEditCopy->setEnabled( FALSE );

    actionEditPaste = new DesignerAction( i18n( "Paste" ), createIconSet("designer_editpaste.png"), i18n( "&Paste" ), CTRL + Key_V, this, 0 );
    actionEditPaste->setStatusTip( i18n( "Pastes the clipboard's contents" ) );
    actionEditPaste->setWhatsThis( whatsThisFrom( "Edit|Paste" ) );
    connect( actionEditPaste, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    actionEditPaste->setEnabled( FALSE );

    actionEditDelete = new DesignerAction( i18n( "Delete" ), QPixmap(), i18n( "&Delete" ), Key_Delete, this, 0 );
    actionEditDelete->setStatusTip( i18n( "Deletes the selected widgets" ) );
    actionEditDelete->setWhatsThis( whatsThisFrom( "Edit|Delete" ) );
    connect( actionEditDelete, SIGNAL( activated() ), this, SLOT( editDelete() ) );
    actionEditDelete->setEnabled( FALSE );
#ifdef Q_WS_MAC
    QAction *macDelete = new DesignerAction( i18n( "Delete" ), QPixmap(), i18n( "&Delete" ), Key_Backspace, this, 0 );
    connect( macDelete, SIGNAL( activated() ), this, SLOT( editDelete() ) );
#endif

    actionEditSelectAll = new DesignerAction( i18n( "Select All" ), QPixmap(), i18n( "Select &All" ), CTRL + Key_A, this, 0 );
    actionEditSelectAll->setStatusTip( i18n( "Selects all widgets" ) );
    actionEditSelectAll->setWhatsThis( whatsThisFrom( "Edit|Select All" ) );
    connect( actionEditSelectAll, SIGNAL( activated() ), this, SLOT( editSelectAll() ) );
    actionEditSelectAll->setEnabled( TRUE );

    actionEditRaise = new DesignerAction( i18n( "Bring to Front" ), createIconSet("designer_editraise.png"), i18n( "Bring to &Front" ), 0, this, 0 );
    actionEditRaise->setStatusTip( i18n( "Raises the selected widgets" ) );
    actionEditRaise->setWhatsThis( i18n( "Raises the selected widgets" ) );
    connect( actionEditRaise, SIGNAL( activated() ), this, SLOT( editRaise() ) );
    actionEditRaise->setEnabled( FALSE );

    actionEditLower = new DesignerAction( i18n( "Send to Back" ), createIconSet("designer_editlower.png"), i18n( "Send to &Back" ), 0, this, 0 );
    actionEditLower->setStatusTip( i18n( "Lowers the selected widgets" ) );
    actionEditLower->setWhatsThis( i18n( "Lowers the selected widgets" ) );
    connect( actionEditLower, SIGNAL( activated() ), this, SLOT( editLower() ) );
    actionEditLower->setEnabled( FALSE );

    actionEditAccels = new DesignerAction( i18n( "Check Accelerators" ), QPixmap(),
				    i18n( "Chec&k Accelerators" ), ALT + Key_R, this, 0 );
    actionEditAccels->setStatusTip( i18n("Checks if the accelerators used in the form are unique") );
    actionEditAccels->setWhatsThis( whatsThisFrom( "Edit|Check Accelerator" ) );
    connect( actionEditAccels, SIGNAL( activated() ), this, SLOT( editAccels() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditAccels, SLOT( setEnabled(bool) ) );

    actionEditFunctions = new DesignerAction( i18n( "Slots" ), createIconSet("designer_editslots.png"),
				   i18n( "S&lots..." ), 0, this, 0 );
    actionEditFunctions->setStatusTip( i18n("Opens a dialog for editing slots") );
    actionEditFunctions->setWhatsThis( whatsThisFrom( "Edit|Slots" ) );
    connect( actionEditFunctions, SIGNAL( activated() ), this, SLOT( editFunctions() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditFunctions, SLOT( setEnabled(bool) ) );

    actionEditConnections = new DesignerAction( i18n( "Connections" ), createIconSet("designer_connecttool.png"),
					 i18n( "Co&nnections..." ), 0, this, 0 );
    actionEditConnections->setStatusTip( i18n("Opens a dialog for editing connections") );
    actionEditConnections->setWhatsThis( whatsThisFrom( "Edit|Connections" ) );
    connect( actionEditConnections, SIGNAL( activated() ), this, SLOT( editConnections() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditConnections, SLOT( setEnabled(bool) ) );

    actionEditSource = new DesignerAction( i18n( "Source" ), QIconSet(),
					 i18n( "&Source..." ), CTRL + Key_E, this, 0 );
    actionEditSource->setStatusTip( i18n("Opens an editor to edit the form's source code") );
    actionEditSource->setWhatsThis( whatsThisFrom( "Edit|Source" ) );
    connect( actionEditSource, SIGNAL( activated() ), this, SLOT( editSource() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditSource, SLOT( setEnabled(bool) ) );

    actionEditFormSettings = new DesignerAction( i18n( "Form Settings" ), QPixmap(),
					  i18n( "&Form Settings..." ), 0, this, 0 );
    actionEditFormSettings->setStatusTip( i18n("Opens a dialog to change the form's settings") );
    actionEditFormSettings->setWhatsThis( whatsThisFrom( "Edit|Form Settings" ) );
    connect( actionEditFormSettings, SIGNAL( activated() ), this, SLOT( editFormSettings() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditFormSettings, SLOT( setEnabled(bool) ) );

    actionEditPreferences = new DesignerAction( i18n( "Preferences" ), QPixmap(),
					 i18n( "Preferences..." ), 0, this, 0 );
    actionEditPreferences->setStatusTip( i18n("Opens a dialog to change preferences") );
    actionEditPreferences->setWhatsThis( whatsThisFrom( "Edit|Preferences" ) );
    connect( actionEditPreferences, SIGNAL( activated() ), this, SLOT( editPreferences() ) );

/*    QToolBar *tb = new QToolBar( this, "Edit" );
    tb->setCloseMode( QDockWindow::Undocked );
    QWhatsThis::add( tb, i18n( "<b>The Edit toolbar</b>%1").arg(i18n(toolbarHelp).arg("")) );
    addToolBar( tb, i18n( "Edit" ) );
    actionEditUndo->addTo( tb );
    actionEditRedo->addTo( tb );
    tb->addSeparator();
    actionEditCut->addTo( tb );
    actionEditCopy->addTo( tb );
    actionEditPaste->addTo( tb );*/
#if 0
    tb->addSeparator();
    actionEditLower->addTo( tb );
    actionEditRaise->addTo( tb );
#endif

    QPopupMenu *menu = new QPopupMenu( this, "Edit" );
    connect( menu, SIGNAL( aboutToShow() ), this, SLOT( updateEditorUndoRedo() ) );
    menubar->insertItem( i18n( "&Edit" ), menu );
    actionEditUndo->addTo( menu );
    actionEditRedo->addTo( menu );
    menu->insertSeparator();
    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );
    actionEditSelectAll->addTo( menu );
    actionEditAccels->addTo( menu );
#if 0
    menu->insertSeparator();
    actionEditLower->addTo( menu );
    actionEditRaise->addTo( menu );
#endif
    menu->insertSeparator();
    if ( !singleProjectMode() ) {
	actionEditFunctions->addTo( menu );
	actionEditConnections->addTo( menu );
    }
    actionEditFormSettings->addTo( menu );
    menu->insertSeparator();
    actionEditPreferences->addTo( menu );
}

void MainWindow::setupSearchActions()
{
    actionSearchFind = new DesignerAction( i18n( "Find" ), createIconSet( "designer_searchfind.png" ),
				    i18n( "&Find..." ), CTRL + Key_F, this, 0 );
    connect( actionSearchFind, SIGNAL( activated() ), this, SLOT( searchFind() ) );
    actionSearchFind->setEnabled( FALSE );
    actionSearchFind->setWhatsThis( whatsThisFrom( "Search|Find" ) );

    actionSearchIncremetal = new DesignerAction( i18n( "Find Incremental" ), QIconSet(),
					  i18n( "Find &Incremental" ), ALT + Key_I, this, 0 );
    connect( actionSearchIncremetal, SIGNAL( activated() ), this, SLOT( searchIncremetalFindMenu() ) );
    actionSearchIncremetal->setEnabled( FALSE );
    actionSearchIncremetal->setWhatsThis( whatsThisFrom( "Search|Find Incremental" ) );

    actionSearchReplace = new DesignerAction( i18n( "Replace" ), QIconSet(),
				    i18n( "&Replace..." ), CTRL + Key_R, this, 0 );
    connect( actionSearchReplace, SIGNAL( activated() ), this, SLOT( searchReplace() ) );
    actionSearchReplace->setEnabled( FALSE );
    actionSearchReplace->setWhatsThis( whatsThisFrom( "Search|Replace" ) );

    actionSearchGotoLine = new DesignerAction( i18n( "Goto Line" ), QIconSet(),
				    i18n( "&Goto Line..." ), ALT + Key_G, this, 0 );
    connect( actionSearchGotoLine, SIGNAL( activated() ), this, SLOT( searchGotoLine() ) );
    actionSearchGotoLine->setEnabled( FALSE );
    actionSearchGotoLine->setWhatsThis( whatsThisFrom( "Search|Goto line" ) );

/*    QToolBar *tb = new QToolBar( this, "Search" );
    tb->setCloseMode( QDockWindow::Undocked );
    addToolBar( tb, i18n( "Search" ) );

    actionSearchFind->addTo( tb );*/
    incrementalSearch = new QLineEdit( 0 );
    incrementalSearch->hide();
    QToolTip::add( incrementalSearch, i18n( "Incremental Search (Alt+I)" ) );
    connect( incrementalSearch, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( searchIncremetalFind() ) );
    connect( incrementalSearch, SIGNAL( returnPressed() ),
	     this, SLOT( searchIncremetalFindNext() ) );
    incrementalSearch->setEnabled( FALSE );

    QPopupMenu *menu = new QPopupMenu( this, "Search" );
    menubar->insertItem( i18n( "&Search" ), menu );
    actionSearchFind->addTo( menu );
    actionSearchIncremetal->addTo( menu );
    actionSearchReplace->addTo( menu );
    menu->insertSeparator();
    actionSearchGotoLine->addTo( menu );
}

void MainWindow::setupLayoutActions()
{
    if ( !actionGroupTools ) {
	actionGroupTools = new QActionGroup( this );
	actionGroupTools->setExclusive( TRUE );
	connect( actionGroupTools, SIGNAL( selected(QAction*) ), this, SLOT( toolSelected(QAction*) ) );
    }

    actionEditAdjustSize = new DesignerAction( i18n( "Adjust Size" ), createIconSet("designer_adjustsize.png"),
					i18n( "Adjust &Size" ), CTRL + Key_J, this, 0 );
    actionEditAdjustSize->setStatusTip(i18n("Adjusts the size of the selected widget") );
    actionEditAdjustSize->setWhatsThis( whatsThisFrom( "Layout|Adjust Size" ) );
    connect( actionEditAdjustSize, SIGNAL( activated() ), this, SLOT( editAdjustSize() ) );
    actionEditAdjustSize->setEnabled( FALSE );

    actionEditHLayout = new DesignerAction( i18n( "Lay Out Horizontally" ), createIconSet("designer_edithlayout.png"),
				     i18n( "Lay Out &Horizontally" ), CTRL + Key_H, this, 0 );
    actionEditHLayout->setStatusTip(i18n("Lays out the selected widgets horizontally") );
    actionEditHLayout->setWhatsThis( whatsThisFrom( "Layout|Lay Out Horizontally" ) );
    connect( actionEditHLayout, SIGNAL( activated() ), this, SLOT( editLayoutHorizontal() ) );
    actionEditHLayout->setEnabled( FALSE );

    actionEditVLayout = new DesignerAction( i18n( "Lay Out Vertically" ), createIconSet("designer_editvlayout.png"),
				     i18n( "Lay Out &Vertically" ), CTRL + Key_L, this, 0 );
    actionEditVLayout->setStatusTip(i18n("Lays out the selected widgets vertically") );
    actionEditVLayout->setWhatsThis(  whatsThisFrom( "Layout|Lay Out Vertically" ) );
    connect( actionEditVLayout, SIGNAL( activated() ), this, SLOT( editLayoutVertical() ) );
    actionEditVLayout->setEnabled( FALSE );

    actionEditGridLayout = new DesignerAction( i18n( "Lay Out in a Grid" ), createIconSet("designer_editgrid.png"),
					i18n( "Lay Out in a &Grid" ), CTRL + Key_G, this, 0 );
    actionEditGridLayout->setStatusTip(i18n("Lays out the selected widgets in a grid") );
    actionEditGridLayout->setWhatsThis( whatsThisFrom( "Layout|Lay Out in a Grid" ) );
    connect( actionEditGridLayout, SIGNAL( activated() ), this, SLOT( editLayoutGrid() ) );
    actionEditGridLayout->setEnabled( FALSE );

    actionEditSplitHorizontal = new DesignerAction( i18n( "Lay Out Horizontally (in Splitter)" ), createIconSet("designer_editvlayoutsplit.png"),
					     i18n( "Lay Out Horizontally (in S&plitter)" ), 0, this, 0 );
    actionEditSplitHorizontal->setStatusTip(i18n("Lays out the selected widgets horizontally in a splitter") );
    actionEditSplitHorizontal->setWhatsThis( whatsThisFrom( "Layout|Lay Out Horizontally (in Splitter)" ) );
    connect( actionEditSplitHorizontal, SIGNAL( activated() ), this, SLOT( editLayoutHorizontalSplit() ) );
    actionEditSplitHorizontal->setEnabled( FALSE );

    actionEditSplitVertical = new DesignerAction( i18n( "Lay Out Vertically (in Splitter)" ), createIconSet("designer_edithlayoutsplit.png"),
					     i18n( "Lay Out Vertically (in Sp&litter)" ), 0, this, 0 );
    actionEditSplitVertical->setStatusTip(i18n("Lays out the selected widgets vertically in a splitter") );
    actionEditSplitVertical->setWhatsThis( whatsThisFrom( "Layout|Lay Out Vertically (in Splitter)" ) );
    connect( actionEditSplitVertical, SIGNAL( activated() ), this, SLOT( editLayoutVerticalSplit() ) );
    actionEditSplitVertical->setEnabled( FALSE );

    actionEditBreakLayout = new DesignerAction( i18n( "Break Layout" ), createIconSet("designer_editbreaklayout.png"),
					 i18n( "&Break Layout" ), CTRL + Key_B, this, 0 );
    actionEditBreakLayout->setStatusTip(i18n("Breaks the selected layout") );
    actionEditBreakLayout->setWhatsThis( whatsThisFrom( "Layout|Break Layout" ) );
    connect( actionEditBreakLayout, SIGNAL( activated() ), this, SLOT( editBreakLayout() ) );
    actionEditBreakLayout->setEnabled( FALSE );

    int id = WidgetDatabase::idFromClassName( "Spacer" );
    actionInsertSpacer = new WidgetAction( i18n( "Layout" ), actionGroupTools, QString::number( id ).latin1() );
    actionInsertSpacer->setToggleAction( TRUE );
    actionInsertSpacer->setText( WidgetDatabase::className( id ) );
    actionInsertSpacer->setMenuText( i18n( "Add %1").arg( WidgetDatabase::className( id ) ) );
    actionInsertSpacer->setIconSet( WidgetDatabase::iconSet( id ) );
    actionInsertSpacer->setToolTip( WidgetDatabase::toolTip( id ) );
    actionInsertSpacer->setStatusTip( i18n( "Insert a %1").arg(WidgetDatabase::toolTip( id )) );
    actionInsertSpacer->setWhatsThis( i18n("<b>A %1</b><p>%2</p>"
			     "<p>Click to insert a single %3,"
			     "or double click to keep the tool selected.")
	.arg(WidgetDatabase::toolTip( id ))
	.arg(WidgetDatabase::whatsThis( id ))
	.arg(WidgetDatabase::toolTip( id ) ));

    actionInsertSpacer->addTo( commonWidgetsToolBar );
    commonWidgetsPage.append( actionInsertSpacer );
    QWidget *w;
    commonWidgetsToolBar->setStretchableWidget( ( w = new QWidget( commonWidgetsToolBar ) ) );
    w->setBackgroundMode( commonWidgetsToolBar->backgroundMode() );
    QWhatsThis::add( layoutToolBar, i18n( "<b>The Layout toolbar</b>%1" ).arg(i18n(toolbarHelp).arg("")) );
    actionEditAdjustSize->addTo( layoutToolBar );
    layoutToolBar->addSeparator();
    actionEditHLayout->addTo( layoutToolBar );
    actionEditVLayout->addTo( layoutToolBar );
    actionEditGridLayout->addTo( layoutToolBar );
    actionEditSplitHorizontal->addTo( layoutToolBar );
    actionEditSplitVertical->addTo( layoutToolBar );
    actionEditBreakLayout->addTo( layoutToolBar );
    layoutToolBar->addSeparator();
    actionInsertSpacer->addTo( layoutToolBar );

    QPopupMenu *menu = new QPopupMenu( this, "Layout" );
    layoutMenu = menu;
    menubar->insertItem( i18n( "&Layout" ), menu, toolsMenuId + 1 );
    actionEditAdjustSize->addTo( menu );
    menu->insertSeparator();
    actionEditHLayout->addTo( menu );
    actionEditVLayout->addTo( menu );
    actionEditGridLayout->addTo( menu );
    actionEditSplitHorizontal->addTo( menu );
    actionEditSplitVertical->addTo( menu );
    actionEditBreakLayout->addTo( menu );
    menu->insertSeparator();
    actionInsertSpacer->addTo( menu );
}

void MainWindow::setupToolActions()
{
    if ( !actionGroupTools ) {
	actionGroupTools = new QActionGroup( this );
	actionGroupTools->setExclusive( TRUE );
	connect( actionGroupTools, SIGNAL( selected(QAction*) ),
		 this, SLOT( toolSelected(QAction*) ) );
    }

    actionPointerTool = new DesignerAction( i18n("Pointer"), createIconSet("designer_pointer.png"),
				     i18n("&Pointer"),  Key_F2,
				     actionGroupTools,
				     QString::number(POINTER_TOOL).latin1(), TRUE );
    actionPointerTool->setStatusTip( i18n("Selects the pointer tool") );
    actionPointerTool->setWhatsThis( whatsThisFrom( "Tools|Pointer" ) );

    actionConnectTool = new DesignerAction( i18n("Connect Signal/Slots"),
				     createIconSet("designer_connecttool.png"),
				     i18n("&Connect Signal/Slots"),
				     singleProjectMode() ? 0 : Key_F3,
				     actionGroupTools,
				     QString::number(CONNECT_TOOL).latin1(), TRUE );
    actionConnectTool->setStatusTip( i18n("Selects the connection tool") );
    actionConnectTool->setWhatsThis( whatsThisFrom( "Tools|Connect Signals and Slots" ) );

    actionOrderTool = new DesignerAction( i18n("Tab Order"), createIconSet("designer_ordertool.png"),
				   i18n("Tab &Order"),  Key_F4,
				   actionGroupTools,
				   QString::number(ORDER_TOOL).latin1(), TRUE );
    actionOrderTool->setStatusTip( i18n("Selects the tab order tool") );
    actionOrderTool->setWhatsThis( whatsThisFrom( "Tools|Tab Order" ) );

    actionBuddyTool = new DesignerAction( i18n( "Set Buddy" ), createIconSet( "designer_setbuddy.png" ),
				   i18n( "Set &Buddy" ), Key_F12,
				   actionGroupTools, QString::number( BUDDY_TOOL ).latin1(),
				   TRUE );
    actionBuddyTool->setStatusTip( i18n( "Sets a buddy to a label" ) );
    actionBuddyTool->setWhatsThis( whatsThisFrom( "Tools|Set Buddy" ) );

    QToolBar *tb = new QToolBar( this, "Tools" );
    tb->setCloseMode( QDockWindow::Undocked );
    toolsToolBar = tb;
    QWhatsThis::add( tb, i18n( "<b>The Tools toolbar</b>%1" ).arg(i18n(toolbarHelp).arg("")) );

    addToolBar( tb, i18n( "Tools" ), QMainWindow::DockTop, FALSE );
    actionPointerTool->addTo( tb );
    if ( !singleProjectMode() )
	actionConnectTool->addTo( tb );
    actionOrderTool->addTo( tb );
    actionBuddyTool->addTo( tb );

    QPopupMenu *mmenu = new QPopupMenu( this, "Tools" );
    toolsMenu = mmenu;
    toolsMenuId = 100;
    menubar->insertItem( i18n( "&Tools" ), mmenu, toolsMenuId );
    toolsMenuIndex = menubar->indexOf( toolsMenuId );
    actionPointerTool->addTo( mmenu );
    if ( !singleProjectMode() )
	actionConnectTool->addTo( mmenu );
    actionOrderTool->addTo( mmenu );
    actionBuddyTool->addTo( mmenu );
    mmenu->insertSeparator();

    customWidgetToolBar = 0;
    customWidgetMenu = 0;

    actionToolsCustomWidget = new DesignerAction( i18n("Custom Widgets"),
					   createIconSet( "designer_customwidget.png" ),
					   i18n("Edit &Custom Widgets..."), 0, this, 0 );
    actionToolsCustomWidget->setStatusTip( i18n("Opens a dialog to add and change "
					      "custom widgets") );
    actionToolsCustomWidget->setWhatsThis( whatsThisFrom( "Tools|Custom|Edit Custom"
							  "Widgets" ) );
    connect( actionToolsCustomWidget, SIGNAL( activated() ),
	     this, SLOT( toolsCustomWidget() ) );

    for ( int j = 0; j < WidgetDatabase::numWidgetGroups(); ++j ) {
	QString grp = WidgetDatabase::widgetGroup( j );
	if ( !WidgetDatabase::isGroupVisible( grp ) ||
	     WidgetDatabase::isGroupEmpty( grp ) )
	    continue;
	QToolBar *tb = new QToolBar( this, grp.latin1() );
	tb->setCloseMode( QDockWindow::Undocked );
	widgetToolBars.append( tb );
	bool plural = grp[(int)grp.length()-1] == 's';
	if ( plural ) {
	    QWhatsThis::add( tb, i18n( "<b>The %1</b>%2" ).arg(grp).arg(i18n(toolbarHelp).
				arg( i18n(" Click on a button to insert a single widget, "
				"or double click to insert multiple %1.") ).arg(grp)) );
	} else {
	    QWhatsThis::add( tb, i18n( "<b>The %1 Widgets</b>%2" ).arg(grp).arg(i18n(toolbarHelp).
				arg( i18n(" Click on a button to insert a single %1 widget, "
				"or double click to insert multiple widgets.") ).arg(grp)) );
	}
	addToolBar( tb, grp );
	tb->hide();
	QPopupMenu *menu = new QPopupMenu( this, grp.latin1() );
	mmenu->insertItem( grp, menu );

	QToolBar *tb2 = new QToolBar( grp, 0, toolBox, FALSE, grp.latin1() );
	tb2->setFrameStyle( QFrame::NoFrame );
	tb2->setOrientation( Qt::Vertical );
	tb2->setBackgroundMode( PaletteBase );
	toolBox->addItem( tb2, grp );

	if ( grp == "Custom" ) {
	    if ( !customWidgetMenu )
		actionToolsCustomWidget->addTo( menu );
	    else
		menu->insertSeparator();
	    customWidgetMenu = menu;
	    customWidgetToolBar = tb;
	}

	for ( int i = 0; i < WidgetDatabase::count(); ++i ) {
	    if ( WidgetDatabase::group( i ) != grp )
		continue; // only widgets, i.e. not forms and temp stuff
	    WidgetAction* a =
		new WidgetAction( grp, actionGroupTools, QString::number( i ).latin1() );
	    a->setToggleAction( TRUE );
	    QString atext = WidgetDatabase::className( i );
	    if ( atext[0] == 'Q' )
		atext = atext.mid(1);
	    while ( atext.length() && atext[0] >= 'a' && atext[0] <= 'z' )
		atext = atext.mid(1);
	    if ( atext.isEmpty() )
		atext = WidgetDatabase::className( i );
	    a->setText( atext );
	    QString ttip = WidgetDatabase::toolTip( i );
	    a->setIconSet( WidgetDatabase::iconSet( i ) );
	    a->setToolTip( ttip );
	    if ( !WidgetDatabase::isWhatsThisLoaded() )
		WidgetDatabase::loadWhatsThis( documentationPath() );
	    a->setStatusTip( i18n( "Insert a %1").arg(WidgetDatabase::className( i )) );

	    QString whats = i18n("<b>A %1</b>").arg( WidgetDatabase::className( i ) );
	    if ( !WidgetDatabase::whatsThis( i ).isEmpty() )
	    whats += QString("<p>%1</p>").arg(WidgetDatabase::whatsThis( i ));
	    a->setWhatsThis( whats+ i18n("<p>Double click on this tool to keep it selected.</p>") );

	    if ( grp != "KDE" )
		a->addTo( tb );
	    a->addTo( menu );
	    a->addTo( tb2 );
	    if ( WidgetDatabase::isCommon( i ) ) {
		a->addTo( commonWidgetsToolBar );
		commonWidgetsPage.append( a );
	    }
	}
	QWidget *w;
	tb2->setStretchableWidget( ( w = new QWidget( tb2 ) ) );
	w->setBackgroundMode( tb2->backgroundMode() );
    }

    if ( !customWidgetToolBar ) {
	QToolBar *tb = new QToolBar( this, "Custom Widgets" );
	tb->setCloseMode( QDockWindow::Undocked );
	QWhatsThis::add( tb, i18n( "<b>The Custom Widgets toolbar</b>%1"
				 "<p>Click <b>Edit Custom Widgets...</b>"
				 "in the <b>Tools|Custom</b> menu to "
				 "add and change custom widgets</p>" ).
			 arg(i18n(toolbarHelp).
			     arg( i18n(" Click on the buttons to insert a single widget, "
				     "or double click to insert multiple widgets.") )) );
	addToolBar( tb, i18n( "Custom" ) );
	tb->hide();
	widgetToolBars.append( tb );
	customWidgetToolBar = tb;
	QPopupMenu *menu = new QPopupMenu( this, "Custom Widgets" );
	mmenu->insertItem( "Custom", menu );
	customWidgetMenu = menu;
	customWidgetToolBar->hide();
	actionToolsCustomWidget->addTo( customWidgetMenu );
	customWidgetMenu->insertSeparator();
	QToolBar *tb2 = new QToolBar( "Custom Widgets", 0,
				      toolBox, FALSE, "Custom Widgets" );
	tb2->setBackgroundMode(PaletteBase);
	tb2->setOrientation( Qt::Vertical );
	tb2->setFrameStyle( QFrame::NoFrame );
	toolBox->addItem( tb2, "Custom Widgets" );
	customWidgetToolBar2 = tb2;
    }

    QAction *a = new DesignerAction( i18n( "Configure Toolbox" ), i18n( "Configure Toolbox..." ), 0, this );
    a->setStatusTip( i18n( "Opens a dialog to configure the common "
					       "widgets page of the toolbox") );
    connect( a, SIGNAL( activated() ), this, SLOT( toolsConfigure() ) );
    mmenu->insertSeparator();
    a->addTo( mmenu );
    resetTool();
}

void MainWindow::setupFileActions()
{
    QToolBar* tb  = new QToolBar( this, "File" );
    tb->setCloseMode( QDockWindow::Undocked );
    projectToolBar = tb;

    QWhatsThis::add( tb, i18n( "<b>The File toolbar</b>%1" ).arg(i18n(toolbarHelp).arg("")) );
    addToolBar( tb, i18n( "File" ) );
    fileMenu = new QPopupMenu( this, "File" );
    menubar->insertItem( i18n( "&File" ), fileMenu );

    DesignerAction *a = 0;

    if ( !singleProject ) {
	DesignerAction *a = new DesignerAction( this, 0 );
	a->setText( i18n( "New" ) );
	a->setToolTip( i18n( "New Dialog or File" ) );
	a->setMenuText( i18n( "&New..." ) );
	a->setIconSet( createIconSet("designer_filenew.png") );
	a->setAccel( CTRL + Key_N );
	a->setStatusTip( i18n( "Creates a new project, form or source file." ) );
	a->setWhatsThis( whatsThisFrom( "File|New" ) );
	connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
	a->addTo( tb );
	a->addTo( fileMenu );
	actionNewFile = a;
    } else {
	actionGroupNew = new QActionGroup( this, 0, FALSE );
	QActionGroup* a = actionGroupNew;
	( (QActionGroup*)a )->setUsesDropDown( TRUE );
	a->setText( i18n( "New" ) );
	a->setMenuText( i18n( "&New..." ) );
	a->setIconSet( createIconSet("designer_form.png") );
	a->setStatusTip( i18n( "Creates a new dialog or file" ) );
	a->setWhatsThis( whatsThisFrom( "File|New" ) );

	QAction *newForm = new DesignerAction( a, 0 );
	newForm->setText( i18n( "New Dialog" ) );
	newForm->setMenuText( i18n( "&Dialog..." ) );
	newForm->setIconSet( createIconSet("designer_form.png") );
	newForm->setAccel( CTRL + Key_N );
	newForm->setStatusTip( i18n( "Creates a new dialog." ) );
	connect( newForm, SIGNAL( activated() ), this, SLOT( fileNewDialog() ) );

	DesignerAction *newFile = new DesignerAction( a, 0 );
	newFile->setText( i18n( "New File" ) );
	newFile->setMenuText( i18n( "&File..." ) );
	newFile->setIconSet( createIconSet("designer_filenew.png") );
	newFile->setAccel( ALT + Key_N );
	newFile->setStatusTip( i18n( "Creates a new file." ) );
	connect( newFile, SIGNAL( activated() ), this, SLOT( fileNewFile() ) );
	actionNewFile = newFile;

	a->addTo( tb );
	a->addTo( fileMenu );

	fileMenu->insertSeparator();
    }

    a = new DesignerAction( this, 0 );
    a->setText( i18n( "Open" ) );
    a->setMenuText( i18n( "&Open..." ) );
    a->setIconSet( createIconSet("designer_fileopen.png") );
    a->setAccel( CTRL + Key_O );
    a->setStatusTip( i18n( "Opens an existing project, form or source file ") );
    a->setWhatsThis( whatsThisFrom( "File|Open" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    if ( !singleProject ) {
	a->addTo( tb );
	a->addTo( fileMenu );
	fileMenu->insertSeparator();
    }


    a = new DesignerAction( this, 0 );
    actionFileClose = a;
    a->setText( i18n( "Close" ) );
    a->setMenuText( i18n( "&Close" ) );
    a->setStatusTip( i18n( "Closes the current project or document" ) );
    a->setWhatsThis(whatsThisFrom( "File|Close" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileClose() ) );
    connect( this, SIGNAL( hasActiveWindowOrProject(bool) ), a, SLOT( setEnabled(bool) ) );
    if ( !singleProject ) {
	a->addTo( fileMenu );
	fileMenu->insertSeparator();
    }

    a = new DesignerAction( this, 0 );
    actionFileSave = a;
    a->setText( i18n( "Save" ) );
    a->setMenuText( i18n( "&Save" ) );
    a->setIconSet( createIconSet("designer_filesave.png") );
    a->setAccel( CTRL + Key_S );
    a->setStatusTip( i18n( "Saves the current project or document" ) );
    a->setWhatsThis(whatsThisFrom( "File|Save" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    connect( this, SIGNAL( hasActiveWindowOrProject(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( tb );
    a->addTo( fileMenu );

    a = new DesignerAction( this, 0 );
    actionFileSaveAs = a;
    a->setText( i18n( "Save As" ) );
    a->setMenuText( i18n( "Save &As..." ) );
    a->setStatusTip( i18n( "Saves the current form with a new filename" ) );
    a->setWhatsThis( whatsThisFrom( "File|Save As" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    connect( this, SIGNAL( hasActiveWindow(bool) ), a, SLOT( setEnabled(bool) ) );
    if ( !singleProject )
	a->addTo( fileMenu );

    a = new DesignerAction( this, 0 );
    actionFileSaveAll = a;
    a->setText( i18n( "Save All" ) );
    a->setMenuText( i18n( "Sa&ve All" ) );
    a->setStatusTip( i18n( "Saves all open documents" ) );
    a->setWhatsThis( whatsThisFrom( "File|Save All" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSaveAll() ) );
    connect( this, SIGNAL( hasActiveWindowOrProject(bool) ), a, SLOT( setEnabled(bool) ) );
    if ( !singleProject ) {
	a->addTo( fileMenu );
	fileMenu->insertSeparator();
    }

    a = new DesignerAction( this, 0 );
    a->setText( i18n( "Create Template" ) );
    a->setMenuText( i18n( "Create &Template..." ) );
    a->setStatusTip( i18n( "Creates a new template" ) );
    a->setWhatsThis( whatsThisFrom( "File|Create Template" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileCreateTemplate() ) );
    if ( !singleProject )
	a->addTo( fileMenu );

    if ( !singleProject )
	fileMenu->insertSeparator();

    recentlyFilesMenu = new QPopupMenu( this );
    recentlyProjectsMenu = new QPopupMenu( this );

    if ( !singleProject ) {
	fileMenu->insertItem( i18n( "Recently Opened Files " ), recentlyFilesMenu );
	fileMenu->insertItem( i18n( "Recently Opened Projects" ), recentlyProjectsMenu );
    }

    connect( recentlyFilesMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( setupRecentlyFilesMenu() ) );
    connect( recentlyProjectsMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( setupRecentlyProjectsMenu() ) );
    connect( recentlyFilesMenu, SIGNAL( activated( int ) ),
	     this, SLOT( recentlyFilesMenuActivated( int ) ) );
    connect( recentlyProjectsMenu, SIGNAL( activated( int ) ),
	     this, SLOT( recentlyProjectsMenuActivated( int ) ) );

    if ( !singleProject )
	fileMenu->insertSeparator();

    a = new DesignerAction( this, 0 );
    actionFileExit = a;
    if  ( !singleProjectMode() ) {
	a->setText( i18n( "Exit" ) );
	a->setMenuText( i18n( "E&xit" ) );
	a->setStatusTip( i18n( "Quits the application and prompts to save any changed forms, source files or project settings" ) );
	a->setWhatsThis( whatsThisFrom( "File|Exit" ) );
    } else {
	a->setText( i18n( "Close" ) );
	a->setMenuText( i18n( "&Close" ) );
    }
    connect( a, SIGNAL( activated() ), this, SLOT( fileQuit() ) );
    a->addTo( fileMenu );
}

void MainWindow::setupProjectActions()
{
    projectMenu = new QPopupMenu( this, "Project" );
    menubar->insertItem( i18n( "Pr&oject" ), projectMenu );

    QActionGroup *ag = new QActionGroup( this, 0 );
    ag->setText( i18n( "Active Project" ) );
    ag->setMenuText( i18n( "Active Project" ) );
    ag->setExclusive( TRUE );
    ag->setUsesDropDown( TRUE );
    connect( ag, SIGNAL( selected( QAction * ) ), this, SLOT( projectSelected( QAction * ) ) );
    connect( ag, SIGNAL( selected( QAction * ) ), this, SIGNAL( projectChanged() ) );
    DesignerAction *a = new DesignerAction( i18n( "<No Project>" ), i18n( "<No Project>" ), 0, ag, 0, TRUE );
    eProject = new Project( "", i18n( "<No Project>" ), projectSettingsPluginManager, TRUE );
    projects.insert( a, eProject );
    a->setOn( TRUE );
    ag->addTo( projectMenu );
    ag->addTo( projectToolBar );
    actionGroupProjects = ag;

    if ( !singleProject )
	projectMenu->insertSeparator();

    a = new DesignerAction( i18n( "Add File" ), QPixmap(), i18n( "&Add File..." ), 0, this, 0 );
    actionProjectAddFile = a;
    a->setStatusTip( i18n("Adds a file to the current project") );
    a->setWhatsThis( whatsThisFrom( "Project|Add File" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( projectInsertFile() ) );
    a->setEnabled( FALSE );
    connect( this, SIGNAL( hasNonDummyProject(bool) ), a, SLOT( setEnabled(bool) ) );
    if ( !singleProject )
	a->addTo( projectMenu );

    actionEditPixmapCollection = new DesignerAction( i18n( "Image Collection..." ), QPixmap(),
					  i18n( "&Image Collection..." ), 0, this, 0 );
    actionEditPixmapCollection->setStatusTip( i18n("Opens a dialog for editing the current project's image collection") );
    actionEditPixmapCollection->setWhatsThis( whatsThisFrom( "Project|Image Collection" ) );
    connect( actionEditPixmapCollection, SIGNAL( activated() ), this, SLOT( editPixmapCollection() ) );
    actionEditPixmapCollection->setEnabled( FALSE );
    connect( this, SIGNAL( hasNonDummyProject(bool) ), actionEditPixmapCollection, SLOT( setEnabled(bool) ) );
    actionEditPixmapCollection->addTo( projectMenu );

#ifndef QT_NO_SQL
    actionEditDatabaseConnections = new DesignerAction( i18n( "Database Connections..." ), QPixmap(),
						 i18n( "&Database Connections..." ), 0, this, 0 );
    actionEditDatabaseConnections->setStatusTip( i18n("Opens a dialog for editing the current project's database connections") );
    actionEditDatabaseConnections->setWhatsThis( whatsThisFrom( "Project|Database Connections" ) );
    connect( actionEditDatabaseConnections, SIGNAL( activated() ), this, SLOT( editDatabaseConnections() ) );
    //actionEditDatabaseConnections->setEnabled( FALSE );
    //connect( this, SIGNAL( hasNonDummyProject(bool) ), actionEditDatabaseConnections, SLOT( setEnabled(bool) ) );
    if ( !singleProject )
	actionEditDatabaseConnections->addTo( projectMenu );
#endif

    actionEditProjectSettings = new DesignerAction( i18n( "Project Settings..." ), QPixmap(),
					  i18n( "&Project Settings..." ), 0, this, 0 );
    actionEditProjectSettings->setStatusTip( i18n("Opens a dialog to change the project's settings") );
    actionEditProjectSettings->setWhatsThis( whatsThisFrom( "Project|Project Settings" ) );
    connect( actionEditProjectSettings, SIGNAL( activated() ), this, SLOT( editProjectSettings() ) );
    actionEditProjectSettings->setEnabled( FALSE );
    connect( this, SIGNAL( hasNonDummyProject(bool) ), actionEditProjectSettings, SLOT( setEnabled(bool) ) );
    actionEditProjectSettings->addTo( projectMenu );

}

void MainWindow::setupPreviewActions()
{
    DesignerAction* a = 0;
    QPopupMenu *menu = new QPopupMenu( this, "Preview" );
    layoutMenu = menu;
    menubar->insertItem( i18n( "&Preview" ), menu, toolsMenuId + 2 );

    a = new DesignerAction( i18n( "Preview Form" ), QPixmap(), i18n( "Preview &Form" ), 0, this, 0 );
    actionPreview = a;
    a->setAccel( CTRL + Key_T );
    a->setStatusTip( i18n("Opens a preview") );
    a->setWhatsThis( whatsThisFrom( "Preview|Preview Form" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( previewForm() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( menu );

    menu->insertSeparator();

    QSignalMapper *mapper = new QSignalMapper( this );
    connect( mapper, SIGNAL(mapped(const QString&)), this, SLOT(previewForm(const QString&)) );
    QStringList styles = QStyleFactory::keys();
    for ( QStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
	QString info;
	if ( *it == "Motif" )
	    info = i18n( "The preview will use the Motif look and feel which is used as the default style on most UNIX systems." );
	else if ( *it == "Windows" )
	    info = i18n( "The preview will use the Windows look and feel." );
	else if ( *it == "Platinum" )
	    info = i18n( "The preview will use the Platinum look and feel which is similar to the Macintosh GUI style." );
	else if ( *it == "CDE" )
	    info = i18n( "The preview will use the CDE look and feel which is similar to some versions of the Common Desktop Environment." );
	else if ( *it == "SGI" )
	    info = i18n( "The preview will use the Motif look and feel which is used as the default style on SGI systems." );
	else if ( *it == "MotifPlus" )
	    info = i18n( "The preview will use the advanced Motif look and feel used by the GIMP toolkit (GTK) on Linux." );

	a = new DesignerAction( i18n( "Preview Form in %1 Style" ).arg( *it ), QPixmap(),
					 i18n( "... in %1 Style" ).arg( *it ), 0, this, 0 );
	a->setStatusTip( i18n("Opens a preview in %1 style").arg( *it ) );
	a->setWhatsThis( i18n("<b>Open a preview in %1 style.</b>"
			"<p>Use the preview to test the design and "
			"signal-slot connections of the current form. %2</p>").arg( *it ).arg( info ) );
	mapper->setMapping( a, *it );
	connect( a, SIGNAL(activated()), mapper, SLOT(map()) );
	connect( this, SIGNAL( hasActiveForm(bool) ), a, SLOT( setEnabled(bool) ) );
	a->addTo( menu );
    }
}

void MainWindow::setupWindowActions()
{
/*    static bool windowActionsSetup = FALSE;
    if ( !windowActionsSetup ) {
	windowActionsSetup = TRUE;
*/
	actionWindowTile = new DesignerAction( i18n( "Tile" ), i18n( "&Tile" ), 0, this );
	actionWindowTile->setStatusTip( i18n("Tiles the windows so that they are all visible") );
	actionWindowTile->setWhatsThis( whatsThisFrom( "Window|Tile" ) );
	connect( actionWindowTile, SIGNAL( activated() ), qworkspace, SLOT( tile() ) );
	actionWindowCascade = new DesignerAction( i18n( "Cascade" ), i18n( "&Cascade" ), 0, this );
	actionWindowCascade->setStatusTip( i18n("Cascades the windows so that all their title bars are visible") );
	actionWindowCascade->setWhatsThis( whatsThisFrom( "Window|Cascade" ) );
	connect( actionWindowCascade, SIGNAL( activated() ), qworkspace, SLOT( cascade() ) );

	actionWindowClose = new DesignerAction( i18n( "Close" ), i18n( "Cl&ose" ), CTRL + Key_F4, this );
	actionWindowClose->setStatusTip( i18n( "Closes the active window") );
	actionWindowClose->setWhatsThis( whatsThisFrom( "Window|Close" ) );
	connect( actionWindowClose, SIGNAL( activated() ), qworkspace, SLOT( closeActiveWindow() ) );

	actionWindowCloseAll = new DesignerAction( i18n( "Close All" ), i18n( "Close Al&l" ), 0, this );
	actionWindowCloseAll->setStatusTip( i18n( "Closes all form windows") );
	actionWindowCloseAll->setWhatsThis( whatsThisFrom( "Window|Close All" ) );
	connect( actionWindowCloseAll, SIGNAL( activated() ), qworkspace, SLOT( closeAllWindows() ) );

	actionWindowNext = new DesignerAction( i18n( "Next" ), i18n( "Ne&xt" ), CTRL + Key_F6, this );
	actionWindowNext->setStatusTip( i18n( "Activates the next window" ) );
	actionWindowNext->setWhatsThis( whatsThisFrom( "Window|Next" ) );
	connect( actionWindowNext, SIGNAL( activated() ), qworkspace, SLOT( activateNextWindow() ) );

	actionWindowPrevious = new DesignerAction( i18n( "Previous" ), i18n( "Pre&vious" ), CTRL + SHIFT + Key_F6, this );
	actionWindowPrevious->setStatusTip( i18n( "Activates the previous window" ) );
	actionWindowPrevious->setWhatsThis( whatsThisFrom( "Window|Previous" ) );
	connect( actionWindowPrevious, SIGNAL( activated() ), qworkspace, SLOT( activatePreviousWindow() ) );
  //  }

    if ( !windowMenu ) {
	windowMenu = new QPopupMenu( this, "Window" );
	menubar->insertItem( i18n( "&Window" ), windowMenu );
	connect( windowMenu, SIGNAL( aboutToShow() ),
		 this, SLOT( setupWindowActions() ) );
    } else {
	windowMenu->clear();
    }

    actionWindowClose->addTo( windowMenu );
    actionWindowCloseAll->addTo( windowMenu );
    windowMenu->insertSeparator();
    actionWindowNext->addTo( windowMenu );
    actionWindowPrevious->addTo( windowMenu );
    windowMenu->insertSeparator();
    actionWindowTile->addTo( windowMenu );
    actionWindowCascade->addTo( windowMenu );
    windowMenu->insertSeparator();
    windowMenu->insertItem( i18n( "Vie&ws" ), createDockWindowMenu( NoToolBars ) );
    windowMenu->insertItem( i18n( "Tool&bars" ), createDockWindowMenu( OnlyToolBars ) );
    QWidgetList windows = qworkspace->windowList();
    if ( windows.count() && formWindow() )
	windowMenu->insertSeparator();
    int j = 0;
    for ( int i = 0; i < int( windows.count() ); ++i ) {
	QWidget *w = windows.at( i );
	if ( !::qt_cast<FormWindow*>(w) && !::qt_cast<SourceEditor*>(w) )
	    continue;
	if ( ::qt_cast<FormWindow*>(w) && ( ( (FormWindow*)w )->isFake() ) )
	    continue;
	j++;
	QString itemText;
	if ( j < 10 )
	    itemText = QString("&%1 ").arg( j );
	if ( ::qt_cast<FormWindow*>(w) )
	    itemText += w->name();
	else
	    itemText += w->caption();

	int id = windowMenu->insertItem( itemText, this, SLOT( windowsMenuActivated( int ) ) );
	windowMenu->setItemParameter( id, i );
	windowMenu->setItemChecked( id, qworkspace->activeWindow() == windows.at( i ) );
    }
}

void MainWindow::setupHelpActions()
{
    actionHelpContents = new DesignerAction( i18n( "Contents" ), i18n( "&Contents" ), Key_F1, this, 0 );
    actionHelpContents->setStatusTip( i18n("Opens the online help") );
    actionHelpContents->setWhatsThis( whatsThisFrom( "Help|Contents" ) );
    connect( actionHelpContents, SIGNAL( activated() ), this, SLOT( helpContents() ) );

    actionHelpManual = new DesignerAction( i18n( "Manual" ), i18n( "&Manual" ), CTRL + Key_M, this, 0 );
    actionHelpManual->setStatusTip( i18n("Opens the Qt Designer manual") );
    actionHelpManual->setWhatsThis( whatsThisFrom( "Help|Manual" ) );
    connect( actionHelpManual, SIGNAL( activated() ), this, SLOT( helpManual() ) );

    actionHelpAbout = new DesignerAction( i18n("About"), QPixmap(), i18n("&About"), 0, this, 0 );
    actionHelpAbout->setStatusTip( i18n("Displays information about Qt Designer") );
    actionHelpAbout->setWhatsThis( whatsThisFrom( "Help|About" ) );
    connect( actionHelpAbout, SIGNAL( activated() ), this, SLOT( helpAbout() ) );

    actionHelpAboutQt = new DesignerAction( i18n("About Qt"), QPixmap(), i18n("About &Qt"), 0, this, 0 );
    actionHelpAboutQt->setStatusTip( i18n("Displays information about the Qt Toolkit") );
    actionHelpAboutQt->setWhatsThis( whatsThisFrom( "Help|About Qt" ) );
    connect( actionHelpAboutQt, SIGNAL( activated() ), this, SLOT( helpAboutQt() ) );

#if 0 //defined(QT_NON_COMMERCIAL)
    // ### not used anymore -- should be deleted?
    actionHelpRegister = new DesignerAction( i18n("Register Qt"), QPixmap(), i18n("&Register Qt..."), 0, this, 0 );
    actionHelpRegister->setStatusTip( i18n("Opens a web browser at the evaluation form on www.trolltech.com") );
    actionHelpRegister->setWhatsThis( i18n("Register with Trolltech") );
    connect( actionHelpRegister, SIGNAL( activated() ), this, SLOT( helpRegister() ) );
#endif

    actionHelpWhatsThis = new DesignerAction( i18n("What's This?"), QIconSet( whatsthis_image, whatsthis_image ),
				       i18n("What's This?"), SHIFT + Key_F1, this, 0 );
    actionHelpWhatsThis->setStatusTip( i18n("\"What's This?\" context sensitive help") );
    actionHelpWhatsThis->setWhatsThis( whatsThisFrom( "Help|What's This?" ) );
    connect( actionHelpWhatsThis, SIGNAL( activated() ), this, SLOT( whatsThis() ) );

/*    QToolBar *tb = new QToolBar( this, "Help" );
    tb->setCloseMode( QDockWindow::Undocked );
    QWhatsThis::add( tb, i18n( "<b>The Help toolbar</b>%1" ).arg(i18n(toolbarHelp).arg("") ));
    addToolBar( tb, i18n( "Help" ) );
    actionHelpWhatsThis->addTo( tb );*/

    QPopupMenu *menu = new QPopupMenu( this, "Help" );
    menubar->insertSeparator();
    menubar->insertItem( i18n( "&Help" ), menu );
    actionHelpContents->addTo( menu );
    actionHelpManual->addTo( menu );
    menu->insertSeparator();
    actionHelpAbout->addTo( menu );
    actionHelpAboutQt->addTo( menu );
#if 0 //defined(QT_NON_COMMERCIAL)
    // ### not used anymore -- should be deleted?
    actionHelpRegister->addTo( menu );
#endif

    menu->insertSeparator();
    actionHelpWhatsThis->addTo( menu );
}

void MainWindow::fileNew()
{
    statusMessage( i18n( "Create a new project, form or source file...") );
    NewForm dlg( this, projectNames(), currentProject->projectName(), templatePath() );
    dlg.exec();
    statusBar()->clear();
}

void MainWindow::fileNewDialog()
{
    static int forms = 0;
    QString n = "Dialog" + QString::number( ++forms );
    while ( currentProject->findFormFile( n + ".ui" ) )
	n = "Dialog" + QString::number( ++forms );
    FormWindow *fw = 0;
    FormFile *ff = new FormFile( n + ".ui", FALSE, currentProject );
    fw = new FormWindow( ff, MainWindow::self, MainWindow::self->qWorkspace(), n );
    ff->setModified( TRUE );
    currentProject->setModified( TRUE );
    workspace()->update();
    fw->setProject( currentProject );
    MetaDataBase::addEntry( fw );
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), fw, n.latin1() );
    fw->setMainContainer( w );
    fw->setCaption( n );
    fw->resize( 600, 480 );
    insertFormWindow( fw );
    fw->killAccels( fw );
    fw->project()->setModified( TRUE );
    fw->setFocus();
    fw->setSavePixmapInProject( TRUE );
    fw->setSavePixmapInline( FALSE );
}

void MainWindow::fileNewFile()
{
    QString name = QInputDialog::getText( i18n( "Name of File" ), i18n( "Enter the name of the new source file:" ) );
    if ( name.isEmpty() )
	return;
    if ( name.right( 3 ) != ".qs" )
	name += ".qs";
    SourceFile *f = new SourceFile( name, FALSE, currentProject );
    MainWindow::self->editSource( f );
    f->setModified( TRUE );
    currentProject->setModified( TRUE );
    workspace()->update();
}

void MainWindow::fileQuit()
{
    close();
    if ( !singleProjectMode() )
	qApp->closeAllWindows();
}

void MainWindow::fileClose()
{
    if ( !currentProject->isDummy() ) {
	fileCloseProject();
    } else {
	QWidget *w = qworkspace->activeWindow();
	if ( w ) {
	    if ( ::qt_cast<FormWindow*>(w) )
		( (FormWindow*)w )->formFile()->close();
	    else if ( ::qt_cast<SourceEditor*>(w) )
		( (SourceEditor*)w )->close();
	}
    }
}


void MainWindow::fileCloseProject()
{
    if ( currentProject->isDummy() )
	return;
    Project *pro = currentProject;
    QAction* a = 0;
    QAction* lastValid = 0;
    for ( QMap<QAction*, Project* >::Iterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( it.data() == pro ) {
	    a = it.key();
	    if ( lastValid )
		break;
	}
	lastValid = it.key();
    }
    if ( a ) {
	if ( pro->isModified() ) {
	    switch ( QMessageBox::warning( this, i18n( "Save Project Settings" ),
					   i18n( "Save changes to '%1'?" ).arg( pro->fileName() ),
					   i18n( "&Yes" ), i18n( "&No" ), i18n( "&Cancel" ), 0, 2 ) ) {
	    case 0: // save
		pro->save();
		break;
	    case 1: // don't save
		break;
	    case 2: // cancel
		return;
	    default:
		break;
	    }
	}

	QWidgetList windows = qWorkspace()->windowList();
	qWorkspace()->blockSignals( TRUE );
	QWidgetListIt wit( windows );
	while ( wit.current() ) {
	    QWidget *w = wit.current();
	    ++wit;
	    if ( ::qt_cast<FormWindow*>(w) ) {
		if ( ( (FormWindow*)w )->project() == pro ) {
		    if ( ( (FormWindow*)w )->formFile()->editor() )
			windows.removeRef( ( (FormWindow*)w )->formFile()->editor() );
		    if ( !( (FormWindow*)w )->formFile()->close() )
			return;
		}
	    } else if ( ::qt_cast<SourceEditor*>(w) ) {
		if ( !( (SourceEditor*)w )->close() )
		    return;
	    }
	}
	hierarchyView->clear();
	windows = qWorkspace()->windowList();
	qWorkspace()->blockSignals( FALSE );
	actionGroupProjects->removeChild( a );
	projects.remove( a );
	delete a;
	currentProject = 0;
	if ( lastValid ) {
	    projectSelected( lastValid );
	    statusMessage( i18n( "Selected project '%1'" ).arg( currentProject->projectName() ) );
	}
	if ( !windows.isEmpty() ) {
	    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
		if ( !::qt_cast<FormWindow*>(w) )
		    continue;
		w->setFocus();
		activeWindowChanged( w );
		break;
	    }
	} else {
	    emit hasActiveWindow( FALSE );
	    emit hasActiveForm( FALSE );
	    updateUndoRedo( FALSE, FALSE, QString::null, QString::null );
	}
    }
}

void MainWindow::fileOpen() // as called by the menu
{
    fileOpen( "", "", "", FALSE );
}

void MainWindow::projectInsertFile()
{
    fileOpen( "", "" );
}

void MainWindow::fileOpen( const QString &filter, const QString &extension, const QString &fn, bool inProject  )
{
    statusMessage( i18n( "Open a file...") );

    QPluginManager<ImportFilterInterface> manager( IID_ImportFilter, QApplication::libraryPaths(), pluginDirectory() );

    Project* project = inProject ? currentProject : eProject;

    QStringList additionalSources;

    {
	QStringList filterlist;
	if ( filter.isEmpty() ) {
	    if ( !inProject )
		filterlist << i18n( "*.ui *.pro|Designer Files" );
	    filterlist << i18n( "*.ui|Qt User-Interface Files" );
	    if ( !inProject )
		filterlist << i18n( "*.pro|QMAKE Project Files" );
	    QStringList list = manager.featureList();
	    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
		filterlist << *it;
	    LanguageInterface *iface = MetaDataBase::languageInterface( project->language() );
	    if ( iface ) {
		filterlist +=  iface->fileFilterList();
		additionalSources += iface->fileExtensionList();
	    }
	    filterlist << i18n( "*|All Files" );
	} else {
	    filterlist << filter;
	}

	QString filters = filterlist.join( "\n" );

	QStringList filenames;
	if ( fn.isEmpty() ) {
	    if ( !inProject ) {
		QString f = KFileDialog::getOpenFileName( QString::null, filters, this, /*0,*/
							  i18n("Open" )/*, &lastOpenFilter */);
		filenames << f;
	    } else {
		filenames = KFileDialog::getOpenFileNames( QString::null, filters, this, /*0,*/
							   i18n("Add")/*, &lastOpenFilter */);
	    }
	} else {
	    filenames << fn;
	}

	for ( QStringList::Iterator fit = filenames.begin(); fit != filenames.end(); ++fit ) {
	    QString filename = *fit;
	    if ( !filename.isEmpty() ) {
		QFileInfo fi( filename );

		if ( fi.extension( FALSE ) == "pro" && ( extension.isEmpty() || extension.find( ";pro" ) != -1 ) ) {
		    addRecentlyOpened( filename, recentlyProjects );
		    openProject( filename );
		} else if ( fi.extension( FALSE ) == "ui" && ( extension.isEmpty() || extension.find( ";ui" ) != -1 ) ) {
		    if ( !inProject )
			setCurrentProject( eProject );
		    openFormWindow( filename );
		    addRecentlyOpened( filename, recentlyFiles );
		} else if ( !extension.isEmpty() && extension.find( ";" + fi.extension( FALSE ) ) != -1 ||
			    additionalSources.find( fi.extension( FALSE ) ) != additionalSources.end() ) {
		    SourceFile *sf = project->findSourceFile( project->makeRelative( filename ) );
		    if ( !sf )
			sf = new SourceFile( project->makeRelative( filename ), FALSE, project );
		    editSource( sf );
		} else if ( extension.isEmpty() ) {
		    QString filter;
		    for ( QStringList::Iterator it2 = filterlist.begin(); it2 != filterlist.end(); ++it2 ) {
			if ( (*it2).contains( "." + fi.extension( FALSE ), FALSE ) ) {
			    filter = *it2;
			    break;
			}
		    }

		    ImportFilterInterface* iface = 0;
		    manager.queryInterface( filter, &iface );
		    if ( !iface ) {
			statusMessage( i18n( "No import filter is available to import '%1'").
					      arg( filename )/*, 3000*/ );
			return;
		    }
		    statusMessage( i18n( "Importing '%1' using import filter ...").arg( filename ) );
		    QStringList list = iface->import( filter, filename );
		    iface->release();
		    if ( list.isEmpty() ) {
			statusMessage( i18n( "Nothing to load in '%1'").arg( filename )/*, 3000*/ );
			return;
		    }
		    if ( !inProject )
			setCurrentProject( eProject );
		    addRecentlyOpened( filename, recentlyFiles );
		    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
			openFormWindow( *it, FALSE );
			QFile::remove( *it );
		    }
		    statusBar()->clear();
		}
	    }
	}
    }
}

FormWindow *MainWindow::openFormWindow( const QString &filename, bool validFileName, FormFile *ff )
{
    if ( filename.isEmpty() )
	return 0;

    bool makeNew = FALSE;

    if ( !QFile::exists( filename ) ) {
	makeNew = TRUE;
    } else {
	QFile f( filename );
	f.open( IO_ReadOnly );
	QTextStream ts( &f );
	makeNew = ts.read().length() < 2;
    }
    if ( makeNew ) {
	fileNew();
	if ( formWindow() )
	    formWindow()->setFileName( filename );
	return formWindow();
    }

    statusMessage( i18n( "Reading file '%1'...").arg( filename ) );
    FormFile *ff2 = currentProject->findFormFile( currentProject->makeRelative(filename) );
    if ( ff2 && ff2->formWindow() ) {
	ff2->formWindow()->setFocus();
	return ff2->formWindow();
    }

    if ( ff2 )
	ff = ff2;
    QApplication::setOverrideCursor( WaitCursor );
    Resource resource( this );
    if ( !ff )
	ff = new FormFile( currentProject->makeRelative( filename ), FALSE, currentProject );
    bool b = resource.load( ff ) && (FormWindow*)resource.widget();
    if ( !validFileName && resource.widget() )
	( (FormWindow*)resource.widget() )->setFileName( QString::null );
    QApplication::restoreOverrideCursor();
    if ( b ) {
	rebuildCustomWidgetGUI();
	statusMessage( i18n( "Loaded file '%1'").arg( filename )/*, 3000 */);
    } else {
	statusMessage( i18n( "Failed to load file '%1'").arg( filename )/*, 5000 */);
	QMessageBox::information( this, i18n("Load File"), i18n("Could not load file '%1'.").arg( filename ) );
	delete ff;
    }
    return (FormWindow*)resource.widget();
}

bool MainWindow::fileSave()
{

    if ( !currentProject->isDummy() )
	return fileSaveProject();
    return fileSaveForm();
}

bool MainWindow::fileSaveForm()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == formWindow() || e == qWorkspace()->activeWindow() ) {
	    e->save();
	}
    }

    FormWindow *fw = 0;

    QWidget *w = qWorkspace()->activeWindow();
    if ( w ) {
	if ( ::qt_cast<SourceEditor*>(w) ) {
	    SourceEditor *se = (SourceEditor*)w;
	    if ( se->formWindow() )
		fw = se->formWindow();
	    else if ( se->sourceFile() ) {
		se->sourceFile()->save();
		return TRUE;
	    }
	}
    }

    if ( !fw )
	fw = formWindow();
    if ( !fw || !fw->formFile()->save() )
	return FALSE;
    QApplication::restoreOverrideCursor();
    return TRUE;
}

bool MainWindow::fileSaveProject()
{
    currentProject->save();
    statusMessage( i18n( "Project '%1' saved.").arg( currentProject->projectName() )/*, 3000 */);
    return TRUE;
}

bool MainWindow::fileSaveAs()
{
    statusMessage( i18n( "Enter a filename..." ) );

    QWidget *w = qworkspace->activeWindow();
    if ( !w )
	return TRUE;
    if ( ::qt_cast<FormWindow*>(w) )
	return ( (FormWindow*)w )->formFile()->saveAs();
    else if ( ::qt_cast<SourceEditor*>(w) )
	return ( (SourceEditor*)w )->saveAs();
    return FALSE;
}

void MainWindow::fileSaveAll()
{
    for ( QMap<QAction*, Project* >::Iterator it = projects.begin(); it != projects.end(); ++it )
	(*it)->save();
}

void MainWindow::fileCreateTemplate()
{
    CreateTemplate dia( this, 0, TRUE );

    int i = 0;
    for ( i = 0; i < WidgetDatabase::count(); ++i ) {
	if ( WidgetDatabase::isForm( i ) && WidgetDatabase::group( i ) != "Temp") {
	    dia.listClass->insertItem( WidgetDatabase::className( i ) );
	}
    }
    for ( i = 0; i < WidgetDatabase::count(); ++i ) {
	if ( WidgetDatabase::isContainer( i ) && !WidgetDatabase::isForm(i) &&
	     WidgetDatabase::className( i ) != "QTabWidget" && WidgetDatabase::group( i ) != "Temp" ) {
	    dia.listClass->insertItem( WidgetDatabase::className( i ) );
	}
    }

    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	if ( w->isContainer )
	    dia.listClass->insertItem( w->className );
    }

    dia.editName->setText( i18n( "NewTemplate" ) );
    connect( dia.buttonCreate, SIGNAL( clicked() ),
	     this, SLOT( createNewTemplate() ) );
    dia.exec();
}

void MainWindow::createNewTemplate()
{
    CreateTemplate *dia = (CreateTemplate*)sender()->parent();
    QString fn = dia->editName->text();
    QString cn = dia->listClass->currentText();
    if ( fn.isEmpty() || cn.isEmpty() ) {
	QMessageBox::information( this, i18n( "Create Template" ), i18n( "Could not create the template." ) );
	return;
    }

    QStringList templRoots;
    const char *qtdir = getenv( "QTDIR" );
    if(qtdir)
	templRoots << qtdir;
    templRoots << qInstallPathData();
    if(qtdir) //try the tools/designer directory last!
	templRoots << (QString(qtdir) + "/tools/designer");
    QFile f;
    for ( QStringList::Iterator it = templRoots.begin(); it != templRoots.end(); ++it ) {
	if ( QFile::exists( (*it) + "/templates/" )) {
	    QString tmpfn = (*it) + "/templates/" + fn + ".ui";
	    f.setName(tmpfn);
	    if(f.open(IO_WriteOnly))
		break;
	}
    }
    if ( !f.isOpen() ) {
	QMessageBox::information( this, i18n( "Create Template" ), i18n( "Could not create the template." ) );
	return;
    }
    QTextStream ts( &f );

    ts << "<!DOCTYPE UI><UI>" << endl;
    ts << "<widget>" << endl;
    ts << "<class>" << cn << "</class>" << endl;
    ts << "<property stdset=\"1\">" << endl;
    ts << "    <name>name</name>" << endl;
    ts << "    <cstring>" << cn << "Form</cstring>" << endl;
    ts << "</property>" << endl;
    ts << "<property stdset=\"1\">" << endl;
    ts << "    <name>geometry</name>" << endl;
    ts << "    <rect>" << endl;
    ts << "        <width>300</width>" << endl;
    ts << "        <height>400</height>" << endl;
    ts << "    </rect>" << endl;
    ts << "</property>" << endl;
    ts << "</widget>" << endl;
    ts << "</UI>" << endl;

    dia->editName->setText( i18n( "NewTemplate" ) );

    f.close();
}

void MainWindow::editUndo()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editUndo();
	return;
    }
    if ( formWindow() )
	formWindow()->undo();
}

void MainWindow::editRedo()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editRedo();
	return;
    }
    if ( formWindow() )
	formWindow()->redo();
}

void MainWindow::editCut()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editCut();
	return;
    }
    editCopy();
    editDelete();
}

void MainWindow::editCopy()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editCopy();
	return;
    }
    if ( formWindow() )
	qApp->clipboard()->setText( formWindow()->copy() );
}

void MainWindow::editPaste()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editPaste();
	return;
    }
    if ( !formWindow() )
	return;

    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 ) {
	w = l.first();
	if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	     ( !WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) &&
	       w != formWindow()->mainContainer() ) )
	    w = formWindow()->mainContainer();
    }

    if ( w && WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
	formWindow()->paste( qApp->clipboard()->text(), WidgetFactory::containerOfWidget( w ) );
	hierarchyView->widgetInserted( 0 );
	formWindow()->commandHistory()->setModified( TRUE );
    } else {
	QMessageBox::information( this, i18n( "Paste Error" ),
				  i18n( "Cannot paste widgets. Designer could not find a container\n"
				      "to paste into which does not contain a layout. Break the layout\n"
				      "of the container you want to paste into and select this container\n"
				      "and then paste again." ) );
    }
}

void MainWindow::editDelete()
{
    if ( formWindow() )
	formWindow()->deleteWidgets();
}

void MainWindow::editSelectAll()
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
	( (SourceEditor*)qWorkspace()->activeWindow() )->editSelectAll();
	return;
    }
    if ( formWindow() )
	formWindow()->selectAll();
}


void MainWindow::editLower()
{
    if ( formWindow() )
	formWindow()->lowerWidgets();
}

void MainWindow::editRaise()
{
    if ( formWindow() )
	formWindow()->raiseWidgets();
}

void MainWindow::editAdjustSize()
{
    if ( formWindow() )
	formWindow()->editAdjustSize();
}

void MainWindow::editLayoutHorizontal()
{
    if ( layoutChilds )
	editLayoutContainerHorizontal();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutHorizontal();
}

void MainWindow::editLayoutVertical()
{
    if ( layoutChilds )
	editLayoutContainerVertical();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutVertical();
}

void MainWindow::editLayoutHorizontalSplit()
{
    if ( layoutChilds )
	; // no way to do that
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutHorizontalSplit();
}

void MainWindow::editLayoutVerticalSplit()
{
    if ( layoutChilds )
	; // no way to do that
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutVerticalSplit();
}

void MainWindow::editLayoutGrid()
{
    if ( layoutChilds )
	editLayoutContainerGrid();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutGrid();
}

void MainWindow::editLayoutContainerVertical()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutVerticalContainer( w  );
}

void MainWindow::editLayoutContainerHorizontal()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutHorizontalContainer( w );
}

void MainWindow::editLayoutContainerGrid()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutGridContainer( w  );
}

void MainWindow::editBreakLayout()
{
    if ( !formWindow() || !breakLayout )
	return;
    QWidget *w = formWindow()->mainContainer();
    if ( formWindow()->currentWidget() )
	w = formWindow()->currentWidget();
    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
	formWindow()->breakLayout( w );
	return;
    } else {
	QWidgetList widgets = formWindow()->selectedWidgets();
	for ( w = widgets.first(); w; w = widgets.next() ) {
	    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
		 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout )
		break;
	}
	if ( w ) {
	    formWindow()->breakLayout( w );
	    return;
	}
    }

    w = formWindow()->mainContainer();
    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout )
	formWindow()->breakLayout( w );
}

void MainWindow::editAccels()
{
    if ( !formWindow() )
	return;
    formWindow()->checkAccels();
}

void MainWindow::editFunctions()
{
    if ( !formWindow() )
	return;

    statusMessage( i18n( "Edit the current form's slots..." ) );
    EditFunctions dlg( this, formWindow(), TRUE );
    dlg.exec();
    statusBar()->clear();
}

void MainWindow::editConnections()
{
    if ( !formWindow() )
	return;

    statusMessage( i18n( "Edit the current form's connections..." ) );
    ConnectionDialog dlg( this );
    dlg.exec();

    statusBar()->clear();
}

SourceEditor *MainWindow::editSource()
{
    if ( !formWindow() )
	return 0;
    return formWindow()->formFile()->showEditor();
}

SourceEditor *MainWindow::openSourceEditor()
{
    if ( !formWindow() )
	return 0;

    QString lang = currentProject->language();
    if ( !MetaDataBase::hasEditor( lang ) ) {
	QMessageBox::information( this, i18n( "Edit Source" ),
				  i18n( "There is no plugin for editing %1 code installed.\n"
				      "Note: Plugins are not available in static Qt configurations." ).arg( lang ) );
	return 0;
    }

    SourceEditor *editor = 0;
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->language() == lang && e->formWindow() == formWindow() ) {
	    editor = e;
	    break;
	}
    }

    if ( !editor )
	editor = createSourceEditor( formWindow(), formWindow()->project(), lang );
    return editor;
}

SourceEditor *MainWindow::editSource( SourceFile *f )
{
    QString lang = currentProject->language();
    if ( !MetaDataBase::hasEditor( lang ) ) {
	QMessageBox::information( this, i18n( "Edit Source" ),
				  i18n( "There is no plugin for editing %1 code installed.\n"
				      "Note: Plugins are not available in static Qt configurations." ).arg( lang ) );
	return 0;
    }

    SourceEditor *editor = 0;
    if ( f )
	editor = f->editor();
    if ( !editor )
	editor = createSourceEditor( f, currentProject, lang );
    editor->setActiveWindow();
    editor->setFocus();
    return editor;
}

SourceEditor *MainWindow::createSourceEditor( QObject *object, Project *project,
					      const QString &lang, const QString &func,
					      bool rereadSource )
{
    SourceEditor *editor = 0;
    EditorInterface *eIface = 0;
    editorPluginManager->queryInterface( lang, &eIface );
    if ( !eIface )
	return 0;
    LanguageInterface *lIface = MetaDataBase::languageInterface( lang );
    if ( !lIface )
	return 0;
    QApplication::setOverrideCursor( WaitCursor );
    editor = new SourceEditor( qWorkspace(), eIface, lIface );
    eIface->release();
    lIface->release();

    editor->setLanguage( lang );
    sourceEditors.append( editor );
    QApplication::restoreOverrideCursor();

    if ( editor->object() != object )
	editor->setObject( object, project );
    else if ( rereadSource )
	editor->refresh( FALSE );

    editor->show();
    editor->setFocus();
    if ( !func.isEmpty() )
	editor->setFunction( func );
    emit editorChanged();

    return editor;
}

void MainWindow::editFormSettings()
{
    if ( !formWindow() )
	return;

    statusMessage( i18n( "Edit the current form's settings..." ) );
    FormSettings dlg( this, formWindow() );
    dlg.exec();
    statusBar()->clear();
}

void MainWindow::editProjectSettings()
{
    openProjectSettings( currentProject );
    wspace->setCurrentProject( currentProject );
}

void MainWindow::editPixmapCollection()
{
    PixmapCollectionEditor dia( this, 0, TRUE );
    dia.setProject( currentProject );
    dia.exec();
}

void MainWindow::editDatabaseConnections()
{
#ifndef QT_NO_SQL
    DatabaseConnectionsEditor dia( currentProject, this, 0, TRUE );
    dia.exec();
#endif
}

void MainWindow::editPreferences()
{
    statusMessage( i18n( "Edit preferences..." ) );
    Preferences *dia = new Preferences( this, 0, TRUE );
    prefDia = dia;
    connect( dia->helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    dia->buttonColor->setEditor( StyledButton::ColorEditor );
    dia->buttonPixmap->setEditor( StyledButton::PixmapEditor );
    dia->groupBoxGrid->setChecked( sGrid );
    dia->checkBoxSnapGrid->setChecked( snGrid );
    dia->spinGridX->setValue( grid().x() );
    dia->spinGridY->setValue( grid().y() );
    dia->checkBoxWorkspace->setChecked( restoreConfig );
    dia->checkBoxTextLabels->setChecked( usesTextLabel() );
    dia->buttonColor->setColor( qworkspace->backgroundColor() );
    QString pluginPaths = QApplication::libraryPaths().join("\n");
    dia->textEditPluginPaths->setText(pluginPaths);
    if ( qworkspace->backgroundPixmap() )
	dia->buttonPixmap->setPixmap( *qworkspace->backgroundPixmap() );
    if ( backPix )
	dia->radioPixmap->setChecked( TRUE );
    else
	dia->radioColor->setChecked( TRUE );
    dia->checkBoxSplash->setChecked( splashScreen );
    dia->checkAutoEdit->setChecked( !databaseAutoEdit );
    dia->checkBoxStartDialog->setChecked( shStartDialog );
    dia->checkBoxAutoSave->setChecked( autoSaveEnabled );
    int h = autoSaveInterval / 3600;
    int m = ( autoSaveInterval - h * 3600 ) / 60;
    int s = autoSaveInterval - ( h * 3600 + m * 60 );
    QTime t( h, m, s );
    dia->timeEditAutoSave->setTime( t );

    SenderObject *senderObject = new SenderObject( designerInterface() );
    QValueList<Tab>::Iterator it;
    for ( it = preferenceTabs.begin(); it != preferenceTabs.end(); ++it ) {
	Tab t = *it;
	dia->tabWidget->addTab( t.w, t.title );
	if ( t.receiver ) {
	    connect( dia->buttonOk, SIGNAL( clicked() ), senderObject, SLOT( emitAcceptSignal() ) );
	    connect( senderObject, SIGNAL( acceptSignal( QUnknownInterface * ) ), t.receiver, t.accept_slot );
	    connect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	    senderObject->emitInitSignal();
	    disconnect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	}
    }

    if ( dia->exec() == QDialog::Accepted ) {
	setSnapGrid( dia->checkBoxSnapGrid->isChecked() );
	setShowGrid( dia->groupBoxGrid->isChecked() );
	setGrid( QPoint( dia->spinGridX->value(),
			 dia->spinGridY->value() ) );
	restoreConfig = dia->checkBoxWorkspace->isChecked();
	setUsesTextLabel( dia->checkBoxTextLabels->isChecked() );
	if (dia->textEditPluginPaths->isModified()) {
	    pluginPaths = dia->textEditPluginPaths->text();
	    QApplication::setLibraryPaths(QStringList::split("\n", pluginPaths));
	    savePluginPaths = TRUE;
	}

	if ( dia->radioPixmap->isChecked() && dia->buttonPixmap->pixmap() ) {
	    qworkspace->setBackgroundPixmap( *dia->buttonPixmap->pixmap() );
	    backPix = TRUE;
	} else {
	    qworkspace->setBackgroundColor( dia->buttonColor->color() );
	    backPix = FALSE;
	}
	splashScreen = dia->checkBoxSplash->isChecked();
	databaseAutoEdit = !dia->checkAutoEdit->isChecked();
	shStartDialog = dia->checkBoxStartDialog->isChecked();
	autoSaveEnabled = dia->checkBoxAutoSave->isChecked();
	QTime time = dia->timeEditAutoSave->time();
	autoSaveInterval = time.hour() * 3600 + time.minute() * 60 + time.second();
	if ( autoSaveEnabled )
	    autoSaveTimer->start( autoSaveInterval * 1000 );
	else
	    autoSaveTimer->stop();
    }
    delete senderObject;
    for ( it = preferenceTabs.begin(); it != preferenceTabs.end(); ++it ) {
	Tab t = *it;
	dia->tabWidget->removePage( t.w );
	t.w->reparent( 0, QPoint(0,0), FALSE );
    }

    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->configChanged();

    delete dia;
    prefDia = 0;
    statusBar()->clear();
}

void MainWindow::searchFind()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return;

    if ( !findDialog )
	findDialog = new FindDialog( this, 0, FALSE );
    findDialog->show();
    findDialog->raise();
    findDialog->setEditor( ( (SourceEditor*)qWorkspace()->activeWindow() )->editorInterface(),
			   ( (SourceEditor*)qWorkspace()->activeWindow() )->object() );
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->selectAll();
}

void MainWindow::searchIncremetalFindMenu()
{
    incrementalSearch->selectAll();
    incrementalSearch->setFocus();
}

void MainWindow::searchIncremetalFind()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return;

    ( (SourceEditor*)qWorkspace()->activeWindow() )->editorInterface()->find( incrementalSearch->text(),
									     FALSE, FALSE, TRUE, FALSE );
}

void MainWindow::searchIncremetalFindNext()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return;

    ( (SourceEditor*)qWorkspace()->activeWindow() )->editorInterface()->find( incrementalSearch->text(),
									     FALSE, FALSE, TRUE, TRUE );
}

void MainWindow::searchReplace()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return;

    if ( !replaceDialog )
	replaceDialog = new ReplaceDialog( this, 0, FALSE );
    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->setEditor( ( (SourceEditor*)qWorkspace()->activeWindow() )->editorInterface(),
			   ( (SourceEditor*)qWorkspace()->activeWindow() )->object() );
    replaceDialog->comboFind->setFocus();
    replaceDialog->comboFind->lineEdit()->selectAll();
}

void MainWindow::searchGotoLine()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return;

    if ( !gotoLineDialog )
	gotoLineDialog = new GotoLineDialog( this, 0, FALSE );
    gotoLineDialog->show();
    gotoLineDialog->raise();
    gotoLineDialog->setEditor( ( (SourceEditor*)qWorkspace()->activeWindow() )->editorInterface() );
    gotoLineDialog->spinLine->setFocus();
    gotoLineDialog->spinLine->setMinValue( 1 );
    gotoLineDialog->spinLine->setMaxValue( ( (SourceEditor*)qWorkspace()->activeWindow() )->numLines() );
    gotoLineDialog->spinLine->selectAll();
}

void MainWindow::toolsCustomWidget()
{
    statusMessage( i18n( "Edit custom widgets..." ) );
    CustomWidgetEditor edit( this, this );
    edit.exec();
    rebuildCustomWidgetGUI();
    statusBar()->clear();
}

void MainWindow::toolsConfigure()
{
    ConfigToolboxDialog dlg( this );
    if ( dlg.exec() != QDialog::Accepted )
	return;
    rebuildCommonWidgetsToolBoxPage();
}

void MainWindow::showStartDialog()
{
    if ( singleProjectMode() )
	return;
    for ( int i = 1; i < qApp->argc(); ++i ) {
	QString arg = qApp->argv()[i];
	if ( arg[0] != '-' )
	    return;
    }
    StartDialog *sd = new StartDialog( this, templatePath() );
    sd->setRecentlyFiles( recentlyFiles );
    sd->setRecentlyProjects( recentlyProjects );
    sd->exec();
    shStartDialog = sd->showDialogInFuture();
}
