/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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

Q_DECLARE_METATYPE( KDevelop::OutputModel::OutputFilterStrategy )

#endif

