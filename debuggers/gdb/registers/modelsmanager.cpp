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

#include "modelsmanager.h"

#include "registercontroller.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QSharedPointer>

#include <KDebug>

namespace GDBDebugger
{

class Converters
{
public:
    static QString formatToString(Format format);

    static Format stringToFormat(const QString& format);
};

QString Converters::formatToString(Format format)
{
    switch (format) {
    case Binary:
        return "Binary";
    case Decimal:
        return "Decimal";
    case Hexadecimal:
        return "Hexadecimal";
    case Natural:
        return "Natural";
    case Octal:
        return "Octal";
    case Raw:
        return "Raw";
    default:
        kDebug() << format;
        Q_ASSERT(0);
        return QString();
    }
}

Format Converters::stringToFormat(const QString& format)
{
    Format def = Raw;

    if (formatToString(Binary) == format) {
        return Binary;
    }
    if (formatToString(Octal) == format) {
        return Octal;
    }
    if (formatToString(Decimal) == format) {
        return Decimal;
    }
    if (formatToString(Hexadecimal) == format) {
        return Hexadecimal;
    }
    for (int i = Raw; i < LAST_FORMAT; i++) {
        if (formatToString(static_cast<Format>(i)) == format) {
            return static_cast<Format>(i);
        }
    }
    return def;
}

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

ModelsManager::ModelsManager(QObject* parent) : QObject(parent), m_models(new Models), m_controller(0) {}

ModelsManager::~ModelsManager() {}

QString ModelsManager::addView(QAbstractItemView* view)
{
    if (m_models->contains(view)) {
        return m_models->nameForView(view);
    }

    Q_ASSERT(m_controller);

    QString name;
    foreach (const GroupsName & group, m_controller->namesOfRegisterGroups()) {
        if (!m_models->contains(group.name())) {
            QStandardItemModel* m = m_models->addModel(Model(group.name(), QSharedPointer<QStandardItemModel>(new QStandardItemModel()), view));
            view->setModel(m);

            //FIXME: receive item's flags as parameters.
            if (group.name() == "Flags") {
                connect(view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(itemChanged(QModelIndex)));
            }

            name = group.name();
            break;
        }
    }

    return name;
}

void ModelsManager::updateModelForGroup(const RegistersGroup& group)
{
    QStandardItemModel* model = m_models->modelForName(group.groupName.name());

    if (!model) {
        return;
    }

    disconnect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));

    model->setRowCount(group.registers.count());
    model->setColumnCount(2);

    int i = 0;

    foreach (const Register & r, group.registers) {
        QStandardItem* n = new QStandardItem(r.name);
        n->setFlags(Qt::ItemIsEnabled);

        QStandardItem* v = new QStandardItem(r.value);
        if (group.flag || !group.editable) {
            v->setFlags(Qt::ItemIsEnabled);
        }

        //FIXME; enable more than 2 columns(for XMM e.t.c.)
        model->setItem(i, 0, n);
        model->setItem(i++, 1, v);
    }

    connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));
}

void ModelsManager::itemChanged(const QModelIndex& idx)
{
    QAbstractItemView* view = static_cast<QAbstractItemView*>(sender());
    int row = idx.row();

    QStandardItemModel* model = m_models->modelForView(view);

    QStandardItem* item = model->item(row, 0);

    Register r;
    r.name = item->text();
    r.value = model->data(idx).toString();
    emit registerChanged(r);
}

QStandardItemModel* Models::addModel(const Model& m)
{
    if (!contains(m.name) && !contains(m.view) && !contains(m.model.data())) {
        m_models.append(m);
        return m.model.data();
    }
    return 0;
}

bool Models::contains(const QString& name)
{
    foreach (const Model & m, m_models) {
        if (m.name == name) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QAbstractItemView* view)
{
    foreach (const Model & m, m_models) {
        if (m.view == view) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QStandardItemModel* model)
{
    foreach (const Model & m, m_models) {
        if (m.model.data() == model) {
            return true;
        }
    }
    return false;
}

QStandardItemModel* Models::modelForName(const QString& name)
{
    foreach (const Model & m, m_models) {
        if (m.name == name) {
            return m.model.data();
        }
    }
    return 0;
}

QStandardItemModel* Models::modelForView(QAbstractItemView* view)
{
    foreach (const Model & m, m_models) {
        if (m.view == view) {
            return m.model.data();
        }
    }
    return 0;
}

void ModelsManager::itemChanged(QStandardItem* i)
{
    QStandardItemModel* model = static_cast<QStandardItemModel*>(sender());
    int row = i->row();

    QStandardItem* name = model->item(row, 0);

    Register r;
    r.name = name->text();
    r.value = i->text();

    emit registerChanged(r);
}

QString Models::nameForView(QAbstractItemView* view)
{
    foreach (const Model & m, m_models) {
        if (m.view == view) {
            return m.name;
        }
    }
    return QString();
}

void ModelsManager::setController(IRegisterController* rc)
{
    m_controller = rc;
    if (!m_controller) {
        m_models->clear();
    } else {
        connect(this, SIGNAL(registerChanged(Register)), m_controller, SLOT(setRegisterValue(Register)));

        connect(m_controller, SIGNAL(registersChanged(RegistersGroup)), this, SLOT(updateModelForGroup(RegistersGroup)));
    }
}

Model::Model() {}

Model::Model(const QString& name, QSharedPointer<QStandardItemModel> model, QAbstractItemView* view)
    : name(name), model(model), view(view) {}

bool Model::operator==(const Model& m)
{
    return m.model == model && m.view == view && m.name == name;
}

void ModelsManager::updateRegisters(const QString& group)
{
    Q_ASSERT(m_controller);
    if (group.isEmpty()) {
        m_controller->updateRegisters(GroupsName());
    } else {
        foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
            if (g.name() == group) {
                m_controller->updateRegisters(g);
                break;
            }
        }
    }
}

void Models::clear()
{
    m_models.clear();
}

void ModelsManager::setFormat(const QString& group, const QString& format)
{
    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            m_controller->setFormat(Converters::stringToFormat(format), g);
            break;
        }
    }
}

QStringList ModelsManager::formats(const QString& group) const
{
    QVector<Format> formats; formats << Raw;
    QStringList l;
    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            formats = m_controller->formats(g);
            break;
        }
    }

    foreach (Format fmt, formats) {
        l << Converters::formatToString(fmt);
    }
    return l;
}

}
