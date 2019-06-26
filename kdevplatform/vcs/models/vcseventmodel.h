/***************************************************************************
 *   This file is part of KDevelop                                         *
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

#ifndef KDEVPLATFORM_VCSEVENTMODEL_H
#define KDEVPLATFORM_VCSEVENTMODEL_H

#include <QAbstractTableModel>
#include <QScopedPointer>

#include <vcs/vcsexport.h>

class QUrl;
class KJob;

namespace KDevelop
{
class VcsRevision;
class IBasicVersionControl;
class VcsEvent;
class VcsEventLogModelPrivate;
class VcsBasicEventModelPrivate;

/**
 * This is a generic model to store a list of VcsEvents.
 *
 * To add events use @c addEvents
 */
class KDEVPLATFORMVCS_EXPORT VcsBasicEventModel : public QAbstractTableModel
{
Q_OBJECT
public:
    enum Column {
        RevisionColumn,
        SummaryColumn,
        AuthorColumn,
        DateColumn,
        ColumnCount,
    };

    explicit VcsBasicEventModel(QObject* parent);
    ~VcsBasicEventModel() override;
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
    QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const override;
    KDevelop::VcsEvent eventForIndex(const QModelIndex&) const;

protected:
    void addEvents(const QList<KDevelop::VcsEvent>&);

private:
    const QScopedPointer<class VcsBasicEventModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsBasicEventModel)
};

/**
 * This model stores a list of VcsEvents corresponding to the log obtained
 * via IBasicVersionControl::log for a given revision. The model is populated
 * lazily via @c fetchMore.
 */
class KDEVPLATFORMVCS_EXPORT VcsEventLogModel : public VcsBasicEventModel
{
Q_OBJECT
public:

    VcsEventLogModel(KDevelop::IBasicVersionControl* iface, const KDevelop::VcsRevision& rev, const QUrl& url, QObject* parent);
    ~VcsEventLogModel() override;

    /// Adds events to the model via @sa IBasicVersionControl::log
    void fetchMore(const QModelIndex& parent) override;
    bool canFetchMore(const QModelIndex& parent) const override;

private Q_SLOTS:
    void jobReceivedResults( KJob* job );

private:
    const QScopedPointer<class VcsEventLogModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsEventLogModel)
};

}

#endif
