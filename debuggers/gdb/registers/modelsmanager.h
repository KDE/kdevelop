/*
 * Class to manage register models.
 * Copyright (C) 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef MODELSMANAGER_H
#define MODELSMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

#include "registercontroller.h"

class QAbstractItemView;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace GDBDebugger
{

struct Model {
    Model();
    Model(const QString& name, QSharedPointer<QStandardItemModel> model, QAbstractItemView* view);
    bool operator==(const Model& m);

    QString name;
    QSharedPointer<QStandardItemModel> model;
    QAbstractItemView* view;
};

class Models
{
public:
    QStandardItemModel* addModel(const Model& m);

    void removeModel(const QString& name);

    void clear();

    bool contains(const QString& name);

    bool contains(QAbstractItemView* view);

    bool contains(QStandardItemModel* model);

    QString nameForView(QAbstractItemView* view);

    ///Returns registered model for @p name, 0 if not registered.
    QStandardItemModel* modelForName(const QString& name);

    ///Returns registered model for @p view, 0 if not registered.
    QStandardItemModel* modelForView(QAbstractItemView* view);

private:
    ///All models
    QVector<Model> m_models;
};

class ModelsManager : public QObject
{
    Q_OBJECT

public:
    explicit ModelsManager(QObject* parent = 0);

    virtual ~ModelsManager();

    ///Adds new @p view with @p name, if not yet registered.
    ///@return: Name of the new view.
    QString addView(QAbstractItemView* view);

    ///Removes existing view with @p name, if any.
    void removeView(const QString& name);

    QStandardItemModel* modelForName(const QString& name);

    void setController(IRegisterController* rc);

    ///Sets @p format for the @p group, if format is valid. Does nothing otherwise.
    void setFormat(const QString& group, RegistersFormat format);//FIXME:

    ///Returns all supported formats for @p group
    QVector<RegistersFormat> formats(const QString& group) const;;//FIXME:

Q_SIGNALS:
    ///Emitted whenever name for view changes(e.g. when architecture changes).
    void nameForViewChanged(const QString& oldName, const QString& newName);

    void registerChanged(const Register&);

public Q_SLOTS:
    void updateModelForGroup(const RegistersGroup& group);

    ///Forcedly updates registers in @p group.
    void updateRegisters(const QString& group = QString());

private Q_SLOTS:
    void itemChanged(const QModelIndex&);

    void itemChanged(QStandardItem*);

private:
    Models m_models;

    IRegisterController* m_controller;
};

}
#endif // MODELSMANAGER_H
