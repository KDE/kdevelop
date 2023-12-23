/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "modelsmanager.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QSharedPointer>

#include <KSharedConfig>

namespace KDevMI {

struct Model {
    Model();
    Model(const QString& name, QSharedPointer<QStandardItemModel> model, QAbstractItemView* view);
    bool operator==(const Model& m) const;

    QString name;
    QSharedPointer<QStandardItemModel> model;
    QAbstractItemView* view = nullptr;
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

} // end of namespace KDevMI

 Q_DECLARE_TYPEINFO(KDevMI::Model, Q_MOVABLE_TYPE);

using namespace KDevMI;

ModelsManager::ModelsManager(QObject* parent)
    : QObject(parent)
    , m_models(new Models)
    , m_config(KSharedConfig::openConfig()->group(QStringLiteral("Register models")))
{}

ModelsManager::~ModelsManager() {}

QString ModelsManager::addView(QAbstractItemView* view)
{
    if (m_models->contains(view)) {
        return m_models->nameForView(view);
    }

    Q_ASSERT(m_controller);

    QString name;
    const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
    for (const GroupsName& group : namesOfRegisterGroups) {
        if (!m_models->contains(group.name())) {
            QStandardItemModel* m = m_models->addModel(Model(group.name(), QSharedPointer<QStandardItemModel>(new QStandardItemModel()), view));
            view->setModel(m);

            if (group.type() == flag) {
                connect(view, &QAbstractItemView::doubleClicked, this, &ModelsManager::flagChanged, Qt::UniqueConnection);
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

    disconnect(model, &QStandardItemModel::itemChanged, this, &ModelsManager::itemChanged);

    model->setRowCount(group.registers.count());
    model->setColumnCount(group.registers.first().value.split(QLatin1Char(' ')).size() + 1);

    //set names and values separately as names don't change so often.
    if (!model->item(0, 0)) {
        for (int row = 0; row < group.registers.count(); row++) {
            const Register& r = group.registers[row];
            auto* n = new QStandardItem(r.name);
            n->setFlags(Qt::ItemIsEnabled);
            model->setItem(row, 0, n);
        }
    }

    for (int row = 0; row < group.registers.count(); row++) {
        const Register& r = group.registers[row];

        const QStringList& values = r.value.split(QLatin1Char(' '));

        //binary format workaround.
        Format currentFormat = formats(group.groupName.name()).first();
        Mode currentMode = modes(group.groupName.name()).first();
        QString prefix;
        if (currentFormat == Binary && ((currentMode < v4_float || currentMode > v2_double) &&
        (currentMode < f32 || currentMode > f64) && group.groupName.type() != floatPoint)) {
            prefix = QStringLiteral("0b");
        }

        for (int column = 0; column  < values.count(); column ++) {
            auto* v = new QStandardItem(prefix + values[column]);
            if (group.groupName.type() == flag) {
                v->setFlags(Qt::ItemIsEnabled);
            }
            model->setItem(row, column + 1, v);
        }
    }

    connect(model, &QStandardItemModel::itemChanged, this, &ModelsManager::itemChanged);
}

void ModelsManager::flagChanged(const QModelIndex& idx)
{
    auto* view = static_cast<QAbstractItemView*>(sender());
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
    return nullptr;
}

bool Models::contains(const QString& name) const
{
    for (const Model &m : m_models) {
        if (m.name == name) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QAbstractItemView* view) const
{
    for (const Model& m : m_models) {
        if (m.view == view) {
            return true;
        }
    }
    return false;
}

bool Models::contains(QStandardItemModel* model) const
{
    for (const Model& m : m_models) {
        if (m.model.data() == model) {
            return true;
        }
    }
    return false;
}

QStandardItemModel* Models::modelForName(const QString& name) const
{
    for (const Model& m : m_models) {
        if (m.name == name) {
            return m.model.data();
        }
    }
    return nullptr;
}

QStandardItemModel* Models::modelForView(QAbstractItemView* view) const
{
    for (const Model& m : m_models) {
        if (m.view == view) {
            return m.model.data();
        }
    }
    return nullptr;
}

void ModelsManager::itemChanged(QStandardItem* i)
{
    auto* model = static_cast<QStandardItemModel*>(sender());

    int row = i->row();

    Register r;
    r.name = model->item(row, 0)->text();
    for (int i = 1; i < model->columnCount(); i++) {
        r.value += model->item(row, i)->text() + QLatin1Char(' ');
    }
    r.value = r.value.trimmed();
    emit registerChanged(r);
}

QString Models::nameForView(QAbstractItemView* view) const
{
    for (const Model& m : m_models) {
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
        connect(this, &ModelsManager::registerChanged, m_controller, &IRegisterController::setRegisterValue);

        connect(m_controller, &IRegisterController::registersChanged, this, &ModelsManager::updateModelForGroup);
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
        const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
        for (const GroupsName& g : namesOfRegisterGroups) {
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
    const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
    for (const GroupsName& g : namesOfRegisterGroups) {
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

    const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
    for (const GroupsName& g : namesOfRegisterGroups) {
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

    const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
    for (const GroupsName& g : namesOfRegisterGroups) {
        if (g.name() == group) {
            modes = m_controller->modes(g);
            break;
        }
    }

    return modes;
}

void ModelsManager::setMode(const QString& group, Mode mode)
{
    const auto namesOfRegisterGroups = m_controller->namesOfRegisterGroups();
    for (const GroupsName& g : namesOfRegisterGroups) {
        if (g.name() == group) {
            m_controller->setMode(mode, g);
            save(g);
            break;
        }
    }
}

#include "moc_modelsmanager.cpp"
