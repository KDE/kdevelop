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
#ifndef _KMDITOOLVIEWACCESSOR_H_
#define _KMDITOOLVIEWACCESSOR_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <kmdidockwidget.h>

class KMdiToolViewAccessor : public QObject
{
   friend class KMdiMainFrm;
   Q_OBJECT

private:
   /**
   * Internally used by KMdiMainFrm to store a temporary information that the method
   * activate() is unnecessary and that it can by escaped.
   * This saves from unnecessary calls when activate is called directly.
   */
   bool m_bInterruptActivation;
   /**
   * Internally used to prevent cycles between KMdiMainFrm::activateView() and KMdiChildView::activate().
   */
   bool m_bMainframesActivateViewIsPending;
   /**
   * 
   */
   bool m_bFocusInEventIsPending;

private:
	KMdiToolViewAccessor( class KMdiMainFrm *parent , QWidget *widgetToWrap);
	KMdiToolViewAccessor( class KMdiMainFrm *parent);
public:
	~KMdiToolViewAccessor();
	QWidget *widgetContainer();
	void setWidget(QWidget* widgetToWrap);
	QWidget *widget();
	void show(KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L,int percent = 50);
	void hide();
private:
   class KMdiToolViewAccessorPrivate *d;
   class KMdiMainFrm *mdiMainFrm;
};


#endif //_KMDITOOLVIEWACCESSOR_H_
