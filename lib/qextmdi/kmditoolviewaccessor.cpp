//----------------------------------------------------------------------------
//    filename             : kmditoolviewaccessor.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 08/2003       by Joseph Wenninger (jowenn@kde.org)
//    changes              : ---
//    patches              : ---
//
//    copyright            : (C) 2003 by Joseph Wenninger (jowenn@kde.org)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef NO_KDE
# include <kdebug.h>
#endif
#include "kmditoolviewaccessor_p.h"
#include "kmdiguiclient.h"
#include "kmdimainfrm.h"

#include "kmditoolviewaccessor.h"
#include "kmditoolviewaccessor_p.h"
//Added by qt3to4:
#include <QPixmap>
#include <QEvent>

KMdiToolViewAccessor::KMdiToolViewAccessor( KMdiMainFrm *parent, QWidget *widgetToWrap, const QString& tabToolTip, const QString& tabCaption)
: QObject(parent) 
{
	mdiMainFrm=parent;
	d=new KMdiToolViewAccessorPrivate();
	if (widgetToWrap->inherits("KDockWidget")) {
		d->widgetContainer=dynamic_cast<KDockWidget*>(widgetToWrap);
		d->widget=d->widgetContainer->getWidget();
	} else {
		d->widget=widgetToWrap;
        QString finalTabCaption;
        if (tabCaption == 0) {
            finalTabCaption = widgetToWrap->caption();
            if (finalTabCaption.isEmpty() && !widgetToWrap->icon()) {
                finalTabCaption = widgetToWrap->name();
            }
        }
        else {
            finalTabCaption = tabCaption;
        }
		d->widgetContainer= parent->createDockWidget( widgetToWrap->name(),
                                              (widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap()),
                                              0L,  // parent
                                              widgetToWrap->caption(),
                                              finalTabCaption);
		d->widgetContainer->setWidget(widgetToWrap);
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	}
	//mdiMainFrm->m_pToolViews->insert(d->widget,this);
	if (mdiMainFrm->m_mdiGUIClient)
		mdiMainFrm->m_mdiGUIClient->addToolView(this);
	else kdDebug(760)<<"mdiMainFrm->m_mdiGUIClient == 0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;

	d->widget->installEventFilter(this);
}

KMdiToolViewAccessor::KMdiToolViewAccessor( KMdiMainFrm *parent) {
	mdiMainFrm=parent;
	d=new KMdiToolViewAccessorPrivate();
}

KMdiToolViewAccessor::~KMdiToolViewAccessor() {
	if (mdiMainFrm->m_pToolViews)
		mdiMainFrm->m_pToolViews->remove(d->widget);
	delete d;

}

QWidget *KMdiToolViewAccessor::wrapperWidget() {
	if (!d->widgetContainer) {
		d->widgetContainer=mdiMainFrm->createDockWidget( "KMdiToolViewAccessor::null",QPixmap());
		connect(d->widgetContainer,SIGNAL(widgetSet(QWidget*)),this,SLOT(setWidgetToWrap(QWidget*)));
	}
	return d->widgetContainer;
}

QWidget *KMdiToolViewAccessor::wrappedWidget() {
	return d->widget;
}


void KMdiToolViewAccessor::setWidgetToWrap(QWidget *widgetToWrap, const QString& tabToolTip, const QString& tabCaption)
{
	Q_ASSERT(!(d->widget));
	Q_ASSERT(!widgetToWrap->inherits("KDockWidget"));
	disconnect(d->widgetContainer,SIGNAL(widgetSet(QWidget*)),this,SLOT(setWidgetToWrap(QWidget*)));
	delete d->widget;
    d->widget=widgetToWrap;
	KDockWidget *tmp=d->widgetContainer;

    QString finalTabCaption;
    if (tabCaption == 0) {
        finalTabCaption = widgetToWrap->caption();
        if (finalTabCaption.isEmpty() && !widgetToWrap->icon()) {
            finalTabCaption = widgetToWrap->name();
        }
    }
    else {
        finalTabCaption = tabCaption;
    }

	if (!tmp) {
		tmp = mdiMainFrm->createDockWidget( widgetToWrap->name(),
			                        widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap(),
			                        0L,  // parent
                                    widgetToWrap->caption(),
                                    finalTabCaption );
		d->widgetContainer= tmp;
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	} 
    else {
		tmp->setCaption(widgetToWrap->caption());
		tmp->setTabPageLabel(finalTabCaption);
		tmp->setPixmap(widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap());
		tmp->setName(widgetToWrap->name());
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	}
	tmp->setWidget(widgetToWrap);
	mdiMainFrm->m_pToolViews->insert(widgetToWrap,this);
	if (mdiMainFrm->m_mdiGUIClient)
		mdiMainFrm->m_mdiGUIClient->addToolView(this);
	else kdDebug(760)<<"mdiMainFrm->m_mdiGUIClient == 0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;

	d->widget->installEventFilter(this);
}


bool KMdiToolViewAccessor::eventFilter(QObject *, QEvent *e) {
	if (e->type()==QEvent::WindowIconChange) {
		d->widgetContainer->setPixmap(d->widget->icon()?(*d->widget->icon()):QPixmap());
	}
	return false;
}

void KMdiToolViewAccessor::placeAndShow(KDockWidget::DockPosition pos, QWidget* pTargetWnd ,int percent)
{
	place(pos,pTargetWnd,percent);
	show();	
}
void KMdiToolViewAccessor::place(KDockWidget::DockPosition pos, QWidget* pTargetWnd ,int percent)
{
    Q_ASSERT(d->widgetContainer);
    if (!d->widgetContainer) return;
    if (pos == KDockWidget::DockNone) {
        d->widgetContainer->setEnableDocking(KDockWidget::DockNone);
        d->widgetContainer->reparent(mdiMainFrm, Qt::WType_TopLevel | Qt::WType_Dialog, QPoint(0,0), true); //pToolView->isVisible());
    }
    else {   // add (and dock) the toolview as DockWidget view
        
        KDockWidget* pCover = d->widgetContainer;
        
        KDockWidget* pTargetDock = 0L;
        if (pTargetWnd->inherits("KDockWidget") || pTargetWnd->inherits("KDockWidget_Compat::KDockWidget")) {
            pTargetDock = (KDockWidget*) pTargetWnd;
        }

        // Should we dock to ourself?
        bool DockToOurself = false;
        if (mdiMainFrm->m_pDockbaseAreaOfDocumentViews) {
            if (pTargetWnd == mdiMainFrm->m_pDockbaseAreaOfDocumentViews->getWidget()) {
                DockToOurself = true;
                pTargetDock = mdiMainFrm->m_pDockbaseAreaOfDocumentViews;
            } 
            else if (pTargetWnd == mdiMainFrm->m_pDockbaseAreaOfDocumentViews) {
                DockToOurself = true;
                pTargetDock = mdiMainFrm->m_pDockbaseAreaOfDocumentViews;
            }
        }
        // this is not inheriting QWidget*, its plain impossible that this condition is true
        //if (pTargetWnd == this) DockToOurself = true;
        if (!DockToOurself) if(pTargetWnd != 0L) {
            pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd);
            if (!pTargetDock) {
                if (pTargetWnd->parentWidget()) {
                    pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd->parentWidget());
                }
            }
        }
        if (!pTargetDock || pTargetWnd == mdiMainFrm->getMainDockWidget()) {
            if (mdiMainFrm->m_managedDockPositionMode && (mdiMainFrm->m_pMdi || mdiMainFrm->m_documentTabWidget)) {
                KDockWidget *dw1=pTargetDock->findNearestDockWidget(pos);
                if (dw1)
                    pCover->manualDock(dw1,KDockWidget::DockCenter,percent);
                else
                    pCover->manualDock ( pTargetDock, pos, 20 );
                return;
            }
        }
        pCover->manualDock( pTargetDock, pos, percent);
//check      pCover->show();
    }
}

void KMdiToolViewAccessor::hide() {
	Q_ASSERT(d->widgetContainer);
	if (!d->widgetContainer) return;
	d->widgetContainer->undock();
}

void KMdiToolViewAccessor::show() {
	Q_ASSERT(d->widgetContainer);
	if (!d->widgetContainer) return;
	d->widgetContainer->makeDockVisible();
}


#ifndef NO_INCLUDE_MOCFILES
#include "kmditoolviewaccessor.moc"
#endif
