/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MODELSMANAGER_H
#define MODELSMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QScopedPointer>

#include <KConfigGroup>

#include "registercontroller.h"

class QAbstractItemView;
class QStandardItem;
class QModelIndex;

namespace KDevMI {
class Models;
class IRegisterController;
struct Register;
struct RegistersGroup;
class GroupsName;

class ModelsManager : public QObject
{
    Q_OBJECT

public:
    explicit ModelsManager(QObject* parent = nullptr);

    ~ModelsManager() override;

    ///Adds new @p view with @p name, if not yet registered.
    ///All views removed after debug session ended.
    ///@return: Name of the new view.
    QString addView(QAbstractItemView* view);

    void setController(IRegisterController* rc);

    ///Sets @p format for the @p group, if format is valid. Does nothing otherwise.
    void setFormat(const QString& group, Format format);

    ///Returns all supported formats for @p group. The first one is current.
    QVector<Format> formats(const QString& group) const;

    ///Sets @p mode for the @p group, if mode is valid. Does nothing otherwise.
    void setMode(const QString& group, Mode mode);

    ///Returns all supported modes for @p group. The first one is current.
    QVector<Mode> modes(const QString& group) const;

Q_SIGNALS:
    ///Emitted when a register in a model changed. Updated value should be send to the debugger.
    void registerChanged(const Register&);

public Q_SLOTS:
    void updateModelForGroup(const RegistersGroup& group);

    ///Forcedly updates registers in @p group.
    void updateRegisters(const QString& group = QString());

private:
    void save(const GroupsName&);
    void load(const GroupsName&);

private Q_SLOTS:
    void flagChanged(const QModelIndex&);

    void itemChanged(QStandardItem*);

private:
    QScopedPointer<Models> m_models;

    IRegisterController* m_controller = nullptr;

    KConfigGroup m_config;
};

} // end of namespace KDevMI
#endif // MODELSMANAGER_H
