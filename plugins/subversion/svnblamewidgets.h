/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include "ui_blameoptiondlg.h"
#include "ui_blamewidget.h"
#include "svnmodels.h"
#include <kdialog.h>
class SvnRevision;
class QSortFilterProxyModel;

class SvnBlameWidget : public QWidget, public Ui::SvnBlameWidget{
public:
    SvnBlameWidget( QWidget *parent );
    virtual ~SvnBlameWidget();
    void refreshWithNewData( const QList<SvnBlameHolder>& datalist );
private:
    SvnBlameModel *m_blameModel;
    QSortFilterProxyModel* m_proxy;
};

// class SvnBlameFileSelectDlg : public KDialog {
//     Q_OBJECT
// public:
//     SvnBlameFileSelectDlg( QWidget *parent = 0L );
//     virtual ~SvnBlameFileSelectDlg();
//     void setCandidate( QStringList *modifies );
//     QString selected();
//
// public Q_SLOTS:
//     virtual void accept();
//
// private:
//     class SvnBlameFileSelectDlgPrivate *d;
// };

class SvnBlameOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnBlameOptionDlg( QWidget *parent = 0 );
    ~SvnBlameOptionDlg();

    SvnRevision startRev();
    SvnRevision endRev();

private:
    Ui::SvnBlameOptionDlg ui;
};


#endif
