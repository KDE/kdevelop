/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 */


#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include <qwidget.h>
#include <qdialog.h>
#include <qvaluelist.h>
class QVBoxLayout;
class QListView;
class QGridLayout;
class QPushButton;
class QStringList;

class SvnBlameHolder {
public:
// 	SvnBlameHolder(){};
// 	~SvnBlameHolder(){};
	unsigned int line;
	long int rev;
	QString date;
	QString author;
	QString content;	
};	

class QListView;

class SvnBlameWidget : public QWidget {
	Q_OBJECT
public:
	SvnBlameWidget( QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags f = 0 );
	virtual ~SvnBlameWidget();
	void copyBlameData( QValueList<SvnBlameHolder> *blamelist );
	void show();
	QListView* outView();
protected:
    QValueList <SvnBlameHolder> m_blamelist;
    
    QVBoxLayout *m_layout;
    QListView *m_listView;
};

class SvnBlameFileSelectDlg : public QDialog {
    Q_OBJECT
public:
    SvnBlameFileSelectDlg( QWidget *parent = 0L );
    virtual ~SvnBlameFileSelectDlg();
    void setCandidate( QStringList *modifies );
    QString selected();
    
protected:
    virtual void accept();
    
private:
    QGridLayout *m_layout;
    QListView *m_view;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    
    QStringList *m_candidates;
    QString m_selected;
};

#endif
