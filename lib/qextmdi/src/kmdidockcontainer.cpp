/* This file is part of the KDE project
   Copyright (C) 2002 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

// $Id$

#include "kmdidockcontainer.h"
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <kmultitabbar.h>

#ifndef NO_KDE
#include <kdebug.h>
#include <kiconloader.h>                       
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#endif

static const char* const not_close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#####",
"#...#",
"#...#",
"#...#",
"#####"};

KMdiDockContainer::KMdiDockContainer(QWidget *parent, QWidget *win, int position):QWidget(parent),KMdiDockContainerBase()
{         
	m_block=false;
	m_inserted=-1;
	m_mainWin = win;
	oldtab=-1;
	mTabCnt=0;
	m_position = position;
	
	QBoxLayout *l;
	m_vertical=!((position==KDockWidget::DockTop) || (position==KDockWidget::DockBottom));

	if (!m_vertical)
	l=new QVBoxLayout(this);
	else
	l=new QHBoxLayout(this);
	
	l->setAutoAdd(false);

	m_tb=new KMultiTabBar(((position==KDockWidget::DockTop) || (position==KDockWidget::DockBottom))?
		KMultiTabBar::Horizontal:KMultiTabBar::Vertical,this);
	m_tb->showActiveTabTexts(true);
	m_tb->setPosition((position==KDockWidget::DockLeft)?KMultiTabBar::Left:
		(position==KDockWidget::DockBottom)?KMultiTabBar::Bottom:
		(position==KDockWidget::DockTop)?KMultiTabBar::Top:KMultiTabBar::Right);

	m_ws=new QWidgetStack(this);

	m_ws->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
 
	if ( (position==KDockWidget::DockLeft) || (position==KDockWidget::DockTop))
	{
		l->add(m_tb);
		l->add(m_ws);
	}
	else
	{
		l->add(m_ws);
		l->add(m_tb);
	}

	l->activate();
	m_ws->hide();

}

KMdiDockContainer::~KMdiDockContainer()
{
}


void KMdiDockContainer::init()
{
	if (m_vertical) {
		parentDockWidget()->setForcedFixedWidth(m_tb->width());	
		activateOverlapMode(m_tb->width());
	}
	else
	{
		parentDockWidget()->setForcedFixedHeight(m_tb->height());
		activateOverlapMode(m_tb->height());
	}

}


KMdiDockWidget *KMdiDockContainer::parentDockWidget(){return ((KMdiDockWidget*)parent());}
    
void KMdiDockContainer::insertWidget (KDockWidget *dwdg, QPixmap pixmap, const QString &text, int &)
{
    KMdiDockWidget* w = (KMdiDockWidget*) dwdg;
	int tab;
	bool alreadyThere=m_map.contains(w);
	if (alreadyThere)
	{
		tab=m_map[w];
		if (m_ws->addWidget(w,tab)!=tab) kdDebug()<<"ERROR COULDN'T READD WIDGET************"<<endl;
		kdDebug()<<"READDED WIDGET***********************************"<<endl;
		m_tb->setTab(tab,true);
		tabClicked(tab);
	}
	else
	{
		tab=m_ws->addWidget(w);
		m_map.insert(w,tab);
		m_revMap.insert(tab,w);

		if (w->getHeader()->qt_cast("KDockWidgetHeader")) {
			kdDebug()<<"*** KDockWidgetHeader has been found"<<endl;
			KDockWidgetHeader *hdr=static_cast<KDockWidgetHeader*>(w->getHeader()->
				qt_cast("KDockWidgetHeader"));
			KDockButton_Private *btn = new KDockButton_Private( hdr, "OverlapButton" );
			QToolTip::add( btn, i18n("Switch between overlap and side by side mode", "Overlap") );
		  	btn->setToggleButton( true );
			btn->setPixmap( const_cast< const char** >(not_close_xpm) );
			hdr->addButton(btn);
			m_overlapButtons.insert(w,btn);
			btn->setOn(!isOverlapMode());
			connect(btn,SIGNAL(clicked()),this,SLOT(changeOverlapMode()));
		}

		m_tb->appendTab(pixmap.isNull()?SmallIcon("misc"):pixmap,tab,w->tabPageLabel());
		kdDebug()<<"NAMENAMENAMENAME:===========================:"<<w->tabPageLabel()<<endl;
		m_tb->setTab(tab,true);
		connect(m_tb->tab(tab),SIGNAL(clicked(int)),this,SLOT(tabClicked(int)));
		kdDebug()<<"KMdiDockContainer::insertWidget()"<<endl;
		m_tb->setTab(oldtab,false);
		mTabCnt++;
		m_inserted=tab;
		int dummy=0;
		tabClicked(tab);
		KDockContainer::insertWidget(w,pixmap,text,dummy);
		itemNames.append(w->name());
	}
        m_ws->raiseWidget(tab);
		
}

void KMdiDockContainer::changeOverlapMode() {
	const KDockButton_Private *btn=dynamic_cast<const KDockButton_Private*>(sender());
	if (!btn) return;
	if (!btn->isOn()) {
		if (m_vertical) {
			activateOverlapMode(m_tb->width());
		}
		else
		{
			activateOverlapMode(m_tb->height());
		}
	} else {
		deactivateOverlapMode();
	}
	
	for (QMap<KMdiDockWidget*,KDockButton_Private*>::iterator it=m_overlapButtons.begin();
		it!=m_overlapButtons.end();++it)
		it.data()->setOn(!isOverlapMode());
}

void KMdiDockContainer::removeWidget(KDockWidget* dwdg)
{
    KMdiDockWidget* w = (KMdiDockWidget*) dwdg;
	if (!m_map.contains(w)) return;
	int id=m_map[w];
	m_tb->setTab(id,false);
	tabClicked(id);
	m_tb->removeTab(id);
	m_map.remove(w);
	m_revMap.remove(id);
	if (m_overlapButtons.contains(w)) {
		(static_cast<KDockWidgetHeader*>(w->getHeader()->qt_cast("KDockWidgetHeader")))->removeButton(m_overlapButtons[w]);
		m_overlapButtons.remove(w);
	}
	KDockContainer::removeWidget(w);
	itemNames.remove(w->name());
}

void KMdiDockContainer::undockWidget(KDockWidget *dwdg)
{
    KMdiDockWidget* w = (KMdiDockWidget*) dwdg;
	if (!m_map.contains(w)) return;
	kdDebug()<<"Wiget has been undocked, setting tab down"<<endl;
	int id=m_map[w];
	m_tb->setTab(id,false);
	tabClicked(id);
}

void KMdiDockContainer::tabClicked(int t)
{

	kdDebug()<<"KMdiDockContainer::tabClicked()"<<endl;

	if (m_tb->isTabRaised(t))
	{         
    if (m_ws->isHidden())
    {                        
       m_ws->show ();
	parentDockWidget()->restoreFromForcedFixedSize();
    }
  		if (!m_ws->widget(t))
		{
			m_revMap[t]->manualDock(parentDockWidget(),KDockWidget::DockCenter,20);
			return;
		}
		m_ws->raiseWidget(t);
		if (m_ws->widget(t)) {
			KDockWidget *tmpDw=static_cast<KDockWidget*>(m_ws->widget(t)->qt_cast("KDockWidget"));
			if (tmpDw) {
				if (tmpDw->getWidget()) tmpDw->getWidget()->setFocus();
			} else kdDebug()<<"Something really weird is going on"<<endl;
		} else
			kdDebug()<<"KMdiDockContainer::tabClicked(int): m_ws->widget(t)==0 "<<endl;

		if (oldtab!=t) m_tb->setTab(oldtab,false);
		oldtab=t;	
	}
	else
	{
//		oldtab=-1;
    if (m_block) return;
    m_block=true;
    if (m_ws->widget(t)) 
    {
//		((KDockWidget*)m_ws->widget(t))->undock();
    }
    m_block=false;
    m_ws->hide ();
	kdDebug()<<"Fixed Width:"<<m_tb->width()<<endl;
	if (m_vertical)
	parentDockWidget()->setForcedFixedWidth(m_tb->width()+2); // strange why it worked before at all
	else
	parentDockWidget()->setForcedFixedHeight(m_tb->height()+2); // strange why it worked before at all
 	}
}

void KMdiDockContainer::setToolTip (KDockWidget *, QString &s)
{
	kdDebug()<<"***********************************Setting tooltip for a widget: "<<s<<endl;
	;
}

#ifndef NO_KDE
void KMdiDockContainer::save(KConfig*)
{
	KConfig *cfg=kapp->config();
	QString grp=cfg->group();
	cfg->deleteGroup(QString("KMdiDock::%1").arg(parent()->name()));
	cfg->setGroup(QString("KMdiDock::%1").arg(parent()->name()));
	
	if (isOverlapMode()) cfg->writeEntry("overlapMode","true");
		else cfg->writeEntry("overlapMode","false");

	QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
	QPtrListIterator<KMultiTabBarTab> it(*tl);
	QStringList::Iterator it2=itemNames.begin();
	int i=0;
	for (;it.current()!=0;++it,++it2)
	{
//		cfg->writeEntry(QString("widget%1").arg(i),m_ws->widget(it.current()->id())->name());	
		cfg->writeEntry(QString("widget%1").arg(i),(*it2));
//		kdDebug()<<"****************************************Saving: "<<m_ws->widget(it.current()->id())->name()<<endl;
		if (m_tb->isTabRaised(it.current()->id()))
			cfg->writeEntry(m_ws->widget(it.current()->id())->name(),true);
	++i;
	}	
	cfg->sync();
	cfg->setGroup(grp);

}
  
void KMdiDockContainer::load(KConfig*)
{
	KConfig *cfg=kapp->config();
	QString grp=cfg->group();	
	cfg->setGroup(QString("KMdiDock::%1").arg(parent()->name()));
	
	if (cfg->readEntry("overlapMode")!="false")
		activateOverlapMode(m_tb->width());
	else
		deactivateOverlapMode();


	int i=0;
	QString raise;
	while (true)
	{
		QString dwn=cfg->readEntry(QString("widget%1").arg(i));
		if (dwn.isEmpty()) break;
		kdDebug()<<"*************************************************************Configuring dockwidget :"<<dwn<<endl;
		KDockWidget *dw=((KDockWidget*)parent())->dockManager()->getDockWidgetFromName(dwn);
		if (dw)
		{
			dw->manualDock((KDockWidget*)parent(),KDockWidget::DockCenter);
		}
		if (cfg->readBoolEntry(dwn,false)) raise=dwn;
		i++;
		
	}
	
	QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
	QPtrListIterator<KMultiTabBarTab> it1(*tl);
	m_ws->hide();
	if (m_vertical)
	parentDockWidget()->setForcedFixedWidth(m_tb->width());
	else
	parentDockWidget()->setForcedFixedHeight(m_tb->height());
	for (;it1.current()!=0;++it1)
	{
		m_tb->setTab(it1.current()->id(),false);
	}
	kapp->syncX();
	m_delayedRaise=-1;
	if (!raise.isEmpty())
	{
		for (QMap<KMdiDockWidget*,int>::iterator it=m_map.begin();it!=m_map.end();++it)
		{

			if (it.key()->name()==raise)
			{
/*				tabClicked(it.data());	
				m_tb->setTab(it.data(),true);
				tabClicked(it.data());	
				m_ws->raiseWidget(it.key());
				kapp->sendPostedEvents();
				kapp->syncX();*/
				m_delayedRaise=it.data();
				QTimer::singleShot(0,this,SLOT(delayedRaise()));
				kdDebug()<<"************** raising *******: "<<it.key()->name()<<endl;
				break;
			}
		}
		
	}
	if (m_delayedRaise==-1) 	QTimer::singleShot(0,this,SLOT(init()));
	cfg->setGroup(grp);
	
}
#endif

void KMdiDockContainer::delayedRaise()
{
				m_tb->setTab(m_delayedRaise,true);
				tabClicked(m_delayedRaise);
}

void KMdiDockContainer::collapseOverlapped()
{
	if (isOverlapMode()){
		QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
	        QPtrListIterator<KMultiTabBarTab> it(*tl);
		for(;it.current();++it) {
			if (it.current()->isOn()) {
				kdDebug()<<"Lowering TAB"<<endl;
				it.current()->setState(false);
				tabClicked(it.current()->id());
			}
		}
	}
}

#ifndef NO_INCLUDE_MOCFILES
#include "kmdidockcontainer.moc"
#endif
