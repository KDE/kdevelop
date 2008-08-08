/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

#include "ui_commitdialog.h"

#include <QtCore/QString>
#include <QtGui/QColor>

#include <KDE/KDialog>

namespace KDevelop
{
    class IDVCSexecutor;
    class VcsStatusInfo;
}

class CommitManager : public KDialog, public Ui::CommitDialogBase
{
    Q_OBJECT
public:
    CommitManager(const QString &_repo, KDevelop::IDVCSexecutor* executor, QWidget *parent = 0);
    ~CommitManager();

    QString commitMessage() const;

private:
    void setCommitCandidates();
    void insertRow(const QList<KDevelop::VcsStatusInfo> statuses,
                   const QString &stType = QString(),
                   const Qt::CheckState ifChecked = Qt::Unchecked);
    void insertRow(const QString& state, const QString& file,
                   const QColor &foregroundColor = Qt::black,
                   const Qt::CheckState ifChecked = Qt::Unchecked);

    QString repo;
    KDevelop::IDVCSexecutor* d;
private slots:
    void commit();
};

#endif
