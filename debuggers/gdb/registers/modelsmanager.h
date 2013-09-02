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
#include <QScopedPointer>

class QAbstractItemView;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace GDBDebugger
{
class Models;
class IRegisterController;
struct Register;
struct RegistersGroup;

class ModelsManager : public QObject
{
    Q_OBJECT

public:
    explicit ModelsManager(QObject* parent = 0);

    virtual ~ModelsManager();

    ///Adds new @p view with @p name, if not yet registered.
    ///All views removed after debug session ended.
    ///@return: Name of the new view.
    QString addView(QAbstractItemView* view);

    void setController(IRegisterController* rc);

    ///Sets @p format for the @p group, if format is valid. Does nothing otherwise.
    void setFormat(const QString& group, const QString& format);

    ///Returns all supported formats for @p group. The first one is current.
    QStringList formats(const QString& group) const;

Q_SIGNALS:
    ///Emitted when a register in a model changed. Updated value should be send to the debugger.
    void registerChanged(const Register&);

public Q_SLOTS:
    void updateModelForGroup(const RegistersGroup& group);

    ///Forcedly updates registers in @p group.
    void updateRegisters(const QString& group = QString());

private Q_SLOTS:
    void flagChanged(const QModelIndex&);

    void itemChanged(QStandardItem*);

private:
    QScopedPointer<Models> m_models;

    IRegisterController* m_controller;
};

}
#endif // MODELSMANAGER_H
