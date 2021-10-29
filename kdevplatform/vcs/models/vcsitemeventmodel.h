/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSITEMEVENTMODEL_H
#define KDEVPLATFORM_VCSITEMEVENTMODEL_H


#include <QStandardItemModel>
#include <vcs/vcsexport.h>

template <typename T> class QList;


namespace KDevelop
{
class VcsItemEvent;

class KDEVPLATFORMVCS_EXPORT VcsItemEventModel : public QStandardItemModel
{
Q_OBJECT
public:
    explicit VcsItemEventModel( QObject* parent );
    ~VcsItemEventModel() override;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void addItemEvents( const QList<KDevelop::VcsItemEvent>& );
    KDevelop::VcsItemEvent itemEventForIndex( const QModelIndex& ) const;
};
}

#endif
