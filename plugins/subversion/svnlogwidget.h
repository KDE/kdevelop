/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef SVNLOGWIDGET_H
#define SVNLOGWIDGET_H

#include <QWidget>
#include <ui_logwidget.h>

namespace KDevelop
{
class VcsJob;
}

class QPoint;
class QModelIndex;
class VcsItemEventModel;
class VcsEventModel;

class SvnLogWidget : public QWidget, public Ui::SvnLogWidget
{
    Q_OBJECT
public:
    explicit SvnLogWidget( const KUrl&, KDevelop::VcsJob*, QWidget *parent = 0 );
    virtual ~SvnLogWidget();

protected slots:
    void diffToPrevious();
    void diffRevisions();
    void jobReceivedResults( KDevelop::VcsJob* );
    void on_eventView_customContextMenuRequested( const QPoint &point );
    void on_eventView_clicked( const QModelIndex &index );
private:
    VcsItemEventModel* m_detailModel;
    VcsEventModel *m_logModel;
    KDevelop::VcsJob* m_job;
    KUrl m_url;
    QModelIndex m_contextIndex;
};


#endif
