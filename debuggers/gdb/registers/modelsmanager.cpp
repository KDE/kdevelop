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

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QSharedPointer>

#include <KSharedConfig>

namespace GDBDebugger
{

struct Model {
    Model();
    Model(const QString& name, QSharedPointer<QStandardItemModel> model, QAbstractItemView* view);
    bool operator==(const Model& m) const;

    QString name;
    QSharedPointer<QStandardItemModel> model;
    QAbstractItemView* view;
};

class Models
{
public:
    QStandardItemModel* addModel(const Model& m);

    void clear();

    bool contains(const QString& name) const;
    bool contains(QAbstractItemView* view) const;
    bool contains(QStandardItemModel* model) const;

    QString nameForView(QAbstractItemView* view) const;

    ///Returns registered model for @p name, 0 if not registered.
    QStandardItemModel* modelForName(const QString& name) const;
    ///Returns registered model for @p view, 0 if not registered.
    QStandardItemModel* modelForView(QAbstractItemView* view) const;

private:
    ///All models
    QVector<Model> m_models;
};

ModelsManager::ModelsManager(QObject* parent) : QObject(parent), m_models(new Models), m_controller(0), m_config(KSharedConfig::openConfig()->group("Register models")) {}

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

            if (group.type() == flag) {
                connect(view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(flagChanged(QModelIndex)), Qt::UniqueConnection);
            }

            name = group.name();
            load(group);
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
    model->setColumnCount(group.registers.first().value.split(' ').size() + 1);

    //set names and values separately as names don't change so often.
    if (!model->item(0, 0)) {
        for (int row = 0; row < group.registers.count(); row++) {
            const Register& r = group.registers[row];
            QStandardItem* n = new QStandardItem(r.name);
            n->setFlags(Qt::ItemIsEnabled);
            model->setItem(row, 0, n);
        }
    }

    for (int row = 0; row < group.registers.count(); row++) {
        const Register& r = group.registers[row];

        const QStringList& values = r.value.split(' ');

        //binary format workaround.
        Format currentFormat = formats(group.groupName.name()).first();
        Mode currentMode = modes(group.groupName.name()).first();
        QString prefix;
        if (currentFormat == Binary && ((currentMode < v4_float || currentMode > v2_double) &&
        (currentMode < f32 || currentMode > f64) && group.groupName.type() != floatPoint)) {
            prefix = "0b";
        }

        for (int column = 0; column  < values.count(); column ++) {
            QStandardItem* v = new QStandardItem(prefix + values[column]);
            if (group.groupName.type() == flag) {
                v->setFlags(Qt::ItemIsEnabled);
            }
            model->setItem(row, column + 1, v);
        }
    }

    connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));
}

void ModelsManager::flagChanged(const QModelIndex& idx)
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

bool Models::contains(const QString& name) const
{
    foreach (const Model & m, m_models) {
        if (m.name == name) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QAbstractItemView* view) const
{
    foreach (const Model & m, m_models) {
        if (m.view == view) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QStandardItemModel* model) const
{
    foreach (const Model & m, m_models) {
        if (m.model.data() == model) {
            return true;
        }
    }
    return false;
}

QStandardItemModel* Models::modelForName(const QString& name) const
{
    foreach (const Model & m, m_models) {
        if (m.name == name) {
            return m.model.data();
        }
    }
    return 0;
}

QStandardItemModel* Models::modelForView(QAbstractItemView* view) const
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

    Register r;
    r.name = model->item(row, 0)->text();
    for (int i = 1; i < model->columnCount(); i++) {
        r.value += model->item(row, i)->text() + ' ';
    }
    r.value = r.value.trimmed();
    emit registerChanged(r);
}

QString Models::nameForView(QAbstractItemView* view) const
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

bool Model::operator==(const Model& m) const
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

void ModelsManager::setFormat(const QString& group, Format format)
{
    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            m_controller->setFormat(format, g);
            save(g);
            break;
        }
    }
}

QVector<Format> ModelsManager::formats(const QString& group) const
{
    QVector<Format> formats; formats << Raw;

    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            formats = m_controller->formats(g);
            break;
        }
    }

    return formats;
}

void ModelsManager::save(const GroupsName& g)
{
    KConfigGroup group = m_config.group(g.name());
    group.writeEntry("format", static_cast<int>(m_controller->formats(g).first()));
    group.writeEntry("mode", static_cast<int>(m_controller->modes(g).first()));
}

void ModelsManager::load(const GroupsName& g)
{
    KConfigGroup group = m_config.group(g.name());

    Format format = static_cast<Format>(group.readEntry("format", static_cast<int>(m_controller->formats(g).first())));
    setFormat(g.name(), format);

    Mode mode = static_cast<Mode>(group.readEntry("mode", static_cast<int>(m_controller->modes(g).first())));
    setMode(g.name(), mode);
}

QVector< Mode > ModelsManager::modes(const QString& group) const
{
    QVector<Mode> modes;

    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            modes = m_controller->modes(g);
            break;
        }
    }

    return modes;
}

void ModelsManager::setMode(const QString& group, Mode mode)
{
    foreach (const GroupsName & g, m_controller->namesOfRegisterGroups()) {
        if (g.name() == group) {
            m_controller->setMode(mode, g);
            save(g);
            break;
        }
    }
}

}
