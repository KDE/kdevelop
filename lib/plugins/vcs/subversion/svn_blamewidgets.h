/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include "ui_svnblamewidget.h"
#include "svn_models.h"
#include <QDialog>

class QListView;
class QGridLayout;
class QPushButton;
class QListWidget;

class SvnBlameWidget : public QWidget, public Ui::SvnBlameWidget{
public:
    SvnBlameWidget( QWidget *parent );
    virtual ~SvnBlameWidget();
    void refreshWithNewData( QList<SvnBlameHolder> datalist );
private:
    BlameItem *m_item;
    BlameTreeModel *m_blameModel;
};

class SvnBlameFileSelectDlg : public QDialog {
    Q_OBJECT
public:
    SvnBlameFileSelectDlg( QWidget *parent = 0L );
    virtual ~SvnBlameFileSelectDlg();
    void setCandidate( QStringList *modifies );
    QString selected();

public Q_SLOTS:
    virtual void accept();

private:
    QGridLayout *m_layout;
    QListWidget *m_listWidget;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;

    QStringList *m_candidates;
    QString m_selected;
};


#endif
