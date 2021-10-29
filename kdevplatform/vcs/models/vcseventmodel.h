/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
