//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _KMDI_DOCUMENT_VIEW_TAB_WIDGET_H_
#define _KMDI_DOCUMENT_VIEW_TAB_WIDGET_H_

#include <ktabwidget.h>
#include "kmdidefines.h"

class KPopupMenu;

//KDE4: Add a d pointer
class KMdiDocumentViewTabWidget:
 public KTabWidget
{
	Q_OBJECT
public:
	KMdiDocumentViewTabWidget(QWidget* parent, const char* name=0);
	~KMdiDocumentViewTabWidget();

	virtual void addTab ( QWidget * child, const QString & label );
	virtual void addTab ( QWidget * child, const QIconSet & iconset, const QString & label );
	virtual void addTab ( QWidget * child, QTab * tab );
	virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );
	virtual void removePage ( QWidget * w );

	KMdi::TabWidgetVisibility tabWidgetVisibility();
	void setTabWidgetVisibility( KMdi::TabWidgetVisibility );

private slots:
	void closeTab(QWidget* w);
public slots:
	void updateIconInView(QWidget*,QPixmap);
	void updateCaptionInView(QWidget*,const QString&);
private:
	void maybeShow();
	KMdi::TabWidgetVisibility m_visibility;
};



#endif
