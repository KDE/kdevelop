/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KMDI_FOCUS_LIST
#define KMDI_FOCUS_LIST

#include <qobject.h>
#include <qmap.h>
#include <qwidget.h>
#include <kdelibs_export.h>

class KMDI_EXPORT KMdiFocusList: public QObject {
	Q_OBJECT
public:
	KMdiFocusList(QObject *parent);
	~KMdiFocusList();
	void addWidgetTree(QWidget*);
	void restore();
protected slots:
	void objectHasBeenDestroyed(QObject*);
private:
	QMap<QWidget*,QWidget::FocusPolicy> m_list;

};

#endif
