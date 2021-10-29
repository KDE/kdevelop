/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OUTPUTMODEL_H
#define KDEVPLATFORM_OUTPUTMODEL_H

#include "outputviewexport.h"
#include "ioutputviewmodel.h"
#include "ifilterstrategy.h"

#include <QAbstractListModel>

class QUrl;

namespace KDevelop
{
class OutputModelPrivate;

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputModel : public QAbstractListModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IOutputViewModel)

public:

    enum CustomRoles {
        OutputItemTypeRole = Qt::UserRole + 1
    };

    enum OutputFilterStrategy
    {
        NoFilter,
        CompilerFilter,
        ScriptErrorFilter,
        NativeAppErrorFilter,
        StaticAnalysisFilter
    };
    Q_ENUM(OutputFilterStrategy)

    explicit OutputModel( const QUrl& builddir , QObject* parent = nullptr );
    explicit OutputModel( QObject* parent = nullptr );
    ~OutputModel() override;

    /// IOutputViewModel interfaces
    void activate( const QModelIndex& index ) override;
    QModelIndex firstHighlightIndex() override;
    QModelIndex nextHighlightIndex( const QModelIndex &current ) override;
    QModelIndex previousHighlightIndex( const QModelIndex &current ) override;
    QModelIndex lastHighlightIndex() override;

    /// QAbstractItemModel interfaces
    QVariant data( const QModelIndex&, int = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex& = QModelIndex() ) const override;
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const override;

    void setFilteringStrategy(const OutputFilterStrategy& currentStrategy);
    void setFilteringStrategy(IFilterStrategy* filterStrategy);

public Q_SLOTS:
    void appendLine( const QString& );
    void appendLines( const QStringList& );
    void ensureAllDone();
    void clear();

Q_SIGNALS:
    /// If the current filter strategy supports it, reports progress information
    void progress(const KDevelop::IFilterStrategy::Progress& progress);
    void allDone();

private:
    const QScopedPointer<class OutputModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(OutputModel)
    friend class OutputModelPrivate;
};

}

#endif
