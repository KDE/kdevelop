/***************************************************************************
*   Copyright (C) 2001 by Bernd Gehrmann                                  *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _FILESELECTORWIDGET_H_
#define _FILESELECTORWIDGET_H_

#include <qlayout.h>
#include <qpushbutton.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <q3strlist.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QFocusEvent>
#include <QDropEvent>

#include <klistview.h>
#include <q3vbox.h>
#include <qwidget.h>
#include <kfile.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kcombobox.h>
#include <kprotocolinfo.h>

#include <kfileview.h>
#include <kfileitem.h>

#include <kdiroperator.h>

class AutoProjectPart;
class SubprojectItem;
class KFile;

class KDnDDirOperator : public KDirOperator
{
	Q_OBJECT

public:
	KDnDDirOperator ( const KURL& urlName = KURL(), QWidget *parent = 0, const char* name = 0 );

protected:
	virtual KFileView* createView( QWidget* parent, KFile::FileView view );
};

class FileSelectorWidget : public QWidget
{
	Q_OBJECT

public:
	FileSelectorWidget( AutoProjectPart* part, KFile::Mode, QWidget* parent = 0, const char* name = 0 );
	~FileSelectorWidget();

	KDnDDirOperator * dirOperator()
	{
		return dir;
	}

public slots:
	void slotFilterChanged( const QString& );
	void setDir( KURL );
	void setDir( const QString& );

private slots:
	void cmbPathActivated( const KURL& u );
	void cmbPathReturnPressed( const QString& u );
	void dirUrlEntered( const KURL& u );
	void dirFinishedLoading();
	void filterReturnPressed( const QString& nf );

protected:
	void focusInEvent( QFocusEvent* );
	void dragEnterEvent ( QDragEnterEvent* ev );
	void dropEvent ( QDropEvent* ev );

private:
	KURLComboBox *cmbPath;
	KHistoryCombo * filter;
	QLabel* filterIcon;
	KDnDDirOperator * dir;
	QPushButton *home, *up, *back, *forward;
	AutoProjectPart* m_part;

signals:
	void dropped ( const QString& );

};

#endif 
// kate: indent-mode csands; tab-width 4;

