/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   based on ktoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kmdiguiclient.h"
#include "kmdiguiclient.moc"

#include <qpopupmenu.h>
#include <kapplication.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kaction.h>
#include <qstring.h>
#include <assert.h>
#include <kdebug.h>
#include <kdockwidget.h>
#include "kmdimainfrm.h"
#include "kmditoolviewaccessor.h"

namespace
{
    const char *actionListName = "show_kmdi_document_tool_view_actions";

    const char *guiDescription = ""
        "<!DOCTYPE kpartgui><kpartgui name=\"KMDIViewActions\">"
        "<MenuBar>"
        "    <Menu name=\"settings\">"
        "        <ActionList name=\"%1\" />"
        "    </Menu>"
        "</MenuBar>"
        "</kpartgui>";

    const char *resourceFileName = "kmdiviewactions.rc";
 
}


using namespace KMDIPrivate;

ToggleToolViewAction::ToggleToolViewAction( const QString& text, const KShortcut& cut,KDockWidget *dw, KMdiMainFrm *mdiMainFrm,
	QObject* parent, const char* name )
        :KToggleAction(text,cut,parent,name),m_dw(dw),m_mdiMainFrm(mdiMainFrm)
{
        connect(this,SIGNAL(toggled(bool)),this,SLOT(slotToggled(bool)));
        connect(m_dw->dockManager(),SIGNAL(change()),this,SLOT(anDWChanged()));
        connect(m_dw,SIGNAL(destroyed()),this,SLOT(slotWidgetDestroyed()));
        setChecked(m_dw->mayBeHide());
}


ToggleToolViewAction::~ToggleToolViewAction(){;}

void ToggleToolViewAction::anDWChanged()
{
        if (isChecked() && m_dw->mayBeShow()) setChecked(false);
        else if ((!isChecked()) && m_dw->mayBeHide()) setChecked(true);
        else if (isChecked() && (m_dw->parentDockTabGroup() &&
                ((static_cast<KDockWidget*>(m_dw->parentDockTabGroup()->
                        parent()->qt_cast("KDockWidget")))->mayBeShow()))) setChecked(false);
}


void ToggleToolViewAction::slotToggled(bool t)
{
//  m_mw->mainDock->setDockSite( KDockWidget::DockCorner );

  if ((!t) && m_dw->mayBeHide() ) m_dw->undock();
  else
    if ( t && m_dw->mayBeShow() ) m_mdiMainFrm->makeDockVisible(m_dw);

//  m_mw->mainDock->setDockSite( KDockWidget::DockNone );
}

void ToggleToolViewAction::slotWidgetDestroyed()

{
        unplugAll();
        deleteLater();
}

KMDIGUIClient::KMDIGUIClient(KMdiMainFrm* mdiMainFrm,const char* name): QObject( mdiMainFrm,name ), KXMLGUIClient( mdiMainFrm )
{
   m_mdiMode=KMdi::ChildframeMode;
   m_mdiMainFrm=mdiMainFrm;
    connect( mdiMainFrm->guiFactory(), SIGNAL( clientAdded( KXMLGUIClient * ) ),
             this, SLOT( clientAdded( KXMLGUIClient * ) ) );

    /* re-use an existing resource file if it exists. can happen if the user launches the
     * toolbar editor */
    /*
    setXMLFile( resourceFileName );
    */

    if ( domDocument().documentElement().isNull() ) {

        QString completeDescription = QString::fromLatin1( guiDescription )
            .arg( actionListName );

        setXML( completeDescription, false /*merge*/ );
    }
 
    m_toolMenu=new KActionMenu(i18n("Tool &Views"),actionCollection(),"kmdi_toolview_menu");
    m_mdiModeAction=new KSelectAction(i18n("MDI Mode"),0,actionCollection());
    QStringList modes;
    modes<<i18n("&Toplevel Mode")<<i18n("C&hildframe Mode")<<i18n("Ta&b Page Mode")<<i18n("I&DEAL Mode");
    m_mdiModeAction->setItems(modes);
    connect(m_mdiModeAction,SIGNAL(activated(int)),this,SLOT(changeViewMode(int)));
    connect(m_mdiMainFrm,SIGNAL(mdiModeHasBeenChangedTo(KMdi::MdiMode)),
	this,SLOT(mdiModeHasBeenChangedTo(KMdi::MdiMode)));

    m_gotoToolDockMenu=new KActionMenu(i18n("Tool &Docks"),actionCollection(),"kmdi_tooldock_menu");
    m_gotoToolDockMenu->insert(new KAction(i18n("Switch Top Dock"),ALT+CTRL+SHIFT+Key_T,this,SIGNAL(toggleTop()),
		actionCollection(),"kmdi_activate_top"));
    m_gotoToolDockMenu->insert(new KAction(i18n("Switch Left Dock"),ALT+CTRL+SHIFT+Key_L,this,SIGNAL(toggleLeft()),
		actionCollection(),"kmdi_activate_left"));
    m_gotoToolDockMenu->insert(new KAction(i18n("Switch Right Dock"),ALT+CTRL+SHIFT+Key_R,this,SIGNAL(toggleRight()),
		actionCollection(),"kmdi_activate_right"));
    m_gotoToolDockMenu->insert(new KAction(i18n("Switch Bottom Dock"),ALT+CTRL+SHIFT+Key_B,this,SIGNAL(toggleBottom()),
		actionCollection(),"kmdi_activate_bottom"));
    m_gotoToolDockMenu->insert(new KActionSeparator(actionCollection(),"kmdi_goto_menu_separator"));
    m_gotoToolDockMenu->insert(new KAction(i18n("Previous Tool View"),ALT+CTRL+Key_Left,m_mdiMainFrm,SLOT(prevToolViewInDock()),
		actionCollection(),"kmdi_next_toolview"));
    m_gotoToolDockMenu->insert(new KAction(i18n("Next Tool View"),ALT+CTRL+Key_Right,m_mdiMainFrm,SLOT(nextToolViewInDock()),
		actionCollection(),"kmdi_next_toolview"));
    


#if 0
   m_pWindowMenu->insertSeparator();
   m_pWindowMenu->insertItem(tr("&MDI Mode..."), m_pMdiModeMenu);
   m_pMdiModeMenu->clear();
   m_pMdiModeMenu->insertItem(tr("&Toplevel Mode"), this, SLOT(switchToToplevelMode()));
   m_pMdiModeMenu->insertItem(tr("C&hildframe Mode"), this, SLOT(switchToChildframeMode()));
   m_pMdiModeMenu->insertItem(tr("Ta&b Page Mode"), this, SLOT(switchToTabPageMode()));
   m_pMdiModeMenu->insertItem(tr("I&DEAl Mode"), this, SLOT(switchToIDEAlMode()));
#endif
}

KMDIGUIClient::~KMDIGUIClient()
{
    m_toolViewActions.setAutoDelete( true );
    m_toolViewActions.clear();
    m_documentViewActions.setAutoDelete( true );
    m_documentViewActions.clear();
}


void KMDIGUIClient::changeViewMode(int id) {
	switch (id) {
		case 0:	m_mdiMainFrm->switchToToplevelMode();
			break;
		case 1: m_mdiMainFrm->switchToChildframeMode();
			break;
		case 2: m_mdiMainFrm->switchToTabPageMode();
			break;
		case 3: m_mdiMainFrm->switchToIDEAlMode();
			break;
		default:
			Q_ASSERT(0);
	}
}

void KMDIGUIClient::setupActions()
{
    if ( !factory() || !m_mdiMainFrm )
        return;

//    BarActionBuilder builder( actionCollection(), m_mainWindow, m_toolBars );

//    if ( !builder.needsRebuild() ) 
//        return;


    unplugActionList( actionListName );

//    m_actions.setAutoDelete( true );
//    m_actions.clear();
//    m_actions.setAutoDelete( false );

//    m_actions = builder.create();

//    m_toolBars = builder.toolBars();

//    m_toolViewActions.append(new KAction( "TESTKMDIGUICLIENT", QString::null, 0,
//             this, SLOT(blah()),actionCollection(),"nothing"));

      QPtrList<KAction> addList;
      if (m_toolViewActions.count()<3) 
	for (int i=0;i<m_toolViewActions.count();i++)
		addList.append(m_toolViewActions.at(i));
      else
        addList.append(m_toolMenu);
      if (m_mdiMode==KMdi::IDEAlMode) addList.append(m_gotoToolDockMenu);
      addList.append(m_mdiModeAction);
      kdDebug()<<"KMDIGUIClient::setupActions: plugActionList"<<endl;
      plugActionList( actionListName, addList );

//    connectToActionContainers();
}

void KMDIGUIClient::addToolView(KMdiToolViewAccessor* mtva) {
	KAction *a=new ToggleToolViewAction(i18n("Show %1").arg(mtva->wrappedWidget()->caption()),
		QString::null,dynamic_cast<KDockWidget*>(mtva->wrapperWidget()),m_mdiMainFrm,actionCollection(),"nothing");
	m_toolViewActions.append(a);
	m_toolMenu->insert(a);
	
	setupActions();
}


void KMDIGUIClient::clientAdded( KXMLGUIClient *client )
{
    if ( client == this )
        setupActions();
}


void KMDIGUIClient::mdiModeHasBeenChangedTo(KMdi::MdiMode mode) {
	kdDebug()<<"KMDIGUIClient::mdiModeHasBennChangeTo"<<endl;
	m_mdiMode=mode;
	switch (mode) {
		case KMdi::ToplevelMode:
			m_mdiModeAction->setCurrentItem(0);
			break;
		case KMdi::ChildframeMode:
			m_mdiModeAction->setCurrentItem(1);
			break;
		case KMdi::TabPageMode:
			m_mdiModeAction->setCurrentItem(2);
			break;
		case KMdi::IDEAlMode:
			m_mdiModeAction->setCurrentItem(3);
			break;
		default: Q_ASSERT(0);
	}
	setupActions();

}

