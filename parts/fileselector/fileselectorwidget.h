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
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <klistview.h>
#include <qvbox.h>
#include <qwidget.h>
#include <kfile.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kcombobox.h>
#include <kprotocolinfo.h>
#include <kdiroperator.h>


class FileSelectorPart;
class KDevProject;


class FileSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    FileSelectorWidget(FileSelectorPart *part);
    ~FileSelectorWidget();

	KDirOperator * dirOperator(){return dir;}

public slots:
	void slotFilterChange(const QString&);
	void setDir(KURL);

private slots:
	void cmbPathActivated( const KURL& u );
	void cmbPathReturnPressed( const QString& u );
	void dirUrlEntered( const KURL& u );
	void dirFinishedLoading();
//	void setCurrentDocDir();

	void popupAboutToShow();
        void popupAboutToHide();
	void popupDelete();


protected:
    void focusInEvent(QFocusEvent*);

private:
    FileSelectorPart *m_part;

	KURLComboBox *cmbPath;
	KHistoryCombo * filter;
	QLabel* filterIcon;
	KDirOperator * dir;
	QPushButton *home, *up, *back, *forward, *cfdir;

    QArray<int> m_popupIds;

    QPopupMenu *m_popup;

};

#endif
