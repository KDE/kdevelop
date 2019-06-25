/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef KDEVPLATFORM_SUBLIMEAGGREGATEMODEL_H
#define KDEVPLATFORM_SUBLIMEAGGREGATEMODEL_H

#include <QAbstractItemModel>
#include "sublimeexport.h"

class QStandardItemModel;

namespace Sublime {

class AggregateModelPrivate;

/**
@short A model to combine several QStandardItemModel's into one.
Combine standard models into the aggregate model to display them in the one view.

Each new model gets its own parent item to differentiate items between different models,
for example:

Tea Model:
@code
- Black
- Green
- White
@endcode
Coffee Model:
@code
- Arabica
- Robusta
@endcode

When aggregated with
@code
    AggregateModel model;
    model->addModel("Tea", teaModel);
    model->addModel("Coffee", coffeeModel);
@endcode
they will look as:
@code
- Tea
    - Black
    - Green
    - White
- Coffee
    - Arabica
    - Robusta
@endcode

@note It is impossible to aggregate any model, aggregation works only for standard models.
@note Currently aggregate model displays only 1 column.
*/
class KDEVPLATFORMSUBLIME_EXPORT AggregateModel: public QAbstractItemModel {
    Q_OBJECT
public:
    explicit AggregateModel(QObject *parent = nullptr);
    ~AggregateModel() override;

    /**Adds the model and creates a parent item with given @p name
    in the aggregated model.*/
    void addModel(const QString &name, QStandardItemModel *model);
    /**Removes the model from aggregation.*/
    void removeModel(QStandardItemModel *model);

    //reimplemented methods from QAbstractItemModel
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

private:
    const QScopedPointer<class AggregateModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AggregateModel)
};

}

#endif

