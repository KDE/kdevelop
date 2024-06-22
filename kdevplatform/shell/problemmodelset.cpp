/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemmodelset.h"

#include "problemmodel.h"

namespace KDevelop
{

class ProblemModelSetPrivate
{
public:
    QVector<ModelData> data;
};

ProblemModelSet::ProblemModelSet(QObject *parent)
    : QObject(parent)
    , d_ptr(new ProblemModelSetPrivate())
{
}

ProblemModelSet::~ProblemModelSet() = default;

void ProblemModelSet::addModel(const QString &id, const QString &name, ProblemModel *model)
{
    Q_D(ProblemModelSet);

    ModelData m{id, name, model};

    d->data.push_back(m);

    connect(model, &ProblemModel::problemsChanged, this, &ProblemModelSet::problemsChanged);

    emit added(m);
}

ProblemModel* ProblemModelSet::findModel(const QString &id) const
{
    Q_D(const ProblemModelSet);

    auto it = std::find_if(d->data.constBegin(), d->data.constEnd(), [&](const ModelData& data) {
        return (data.id == id);
    });

    return (it != d->data.constEnd()) ? it->model : nullptr;
}

void ProblemModelSet::removeModel(const QString &id)
{
    Q_D(ProblemModelSet);

    QVector<ModelData>::iterator itr = d->data.begin();

    while (itr != d->data.end()) {
        if(itr->id == id)
            break;
        ++itr;
    }

    if(itr != d->data.end()) {
        (*itr).model->disconnect(this);
        d->data.erase(itr);
        emit removed(id);
    }
}

void ProblemModelSet::showModel(const QString &id)
{
    Q_D(ProblemModelSet);

    for (const ModelData& data : std::as_const(d->data)) {
        if (data.id == id) {
            emit showRequested(data.id);
            return;
        }
    }
}

QVector<ModelData> ProblemModelSet::models() const
{
    Q_D(const ProblemModelSet);

    return d->data;
}

}

#include "moc_problemmodelset.cpp"
