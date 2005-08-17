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
#include <kmdidefines.h>
//Added by qt3to4:
#include <QPixmap>

class KPopupMenu;

//KDE4: Add a d pointer
/**
 * A reimplementation of KTabWidget for KMDI
 */
class KMDI_EXPORT KMdiDocumentViewTabWidget:
 public KTabWidget
{
	Q_OBJECT
public:
	KMdiDocumentViewTabWidget(QWidget* parent, const char* name=0);
	~KMdiDocumentViewTabWidget();

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, const QString & label );

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, const QIcon & iconset, const QString & label );

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, QTab * tab );

	/**
	 * Insert a tab into the tabwidget with a label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );

	/**
	 * Inserts a tab into the tabwidget with an icon and label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, const QIcon & iconset, const QString & label, int index = -1 );

	/**
	 * Inserts a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );

	/**
	 * Removes the tab from the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void removePage ( QWidget * w );

	/**
	 * Set the tab widget's visibility and then make the change
	 * to match the new setting
	 */
	KMdi::TabWidgetVisibility tabWidgetVisibility();

	/** Get the tab widget's visibility */
	void setTabWidgetVisibility( KMdi::TabWidgetVisibility );

private slots:

	/** Close the tab specified by w */
	void closeTab(QWidget* w);

public slots:

	/** Change the icon for the tab */
	void updateIconInView(QWidget*,QPixmap);

	/** Change the caption for the tab */
	void updateCaptionInView(QWidget*,const QString&);

private:

	/**
	 * Determine whether or not we should show the tab bar
	 * The tab bar is hidden if it's determined that it should be hidden
	 * and shown if it's determined that it should be shown
	 */
	void maybeShow();
	
	KMdi::TabWidgetVisibility m_visibility;
};



#endif
