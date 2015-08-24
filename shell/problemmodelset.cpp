/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "problemmodelset.h"
#include <QAction>

namespace KDevelop
{

struct ProblemModelSetPrivate
{
    QVector<ModelData> data;
};

ProblemModelSet::ProblemModelSet(QObject *parent)
    : QObject(parent)
    , d(new ProblemModelSetPrivate())
{
}

ProblemModelSet::~ProblemModelSet() = default;

void ProblemModelSet::addModel(const QString &name, ProblemModel *model)
{
    ModelData m;
    m.name = name;
    m.model = model;
    d->data.push_back(m);

    emit added(m);
}

ProblemModel* ProblemModelSet::findModel(const QString &name) const
{
    ProblemModel *model = nullptr;

    foreach (const ModelData &data, d->data) {
        if (data.name == name) {
            model = data.model;
            break;
        }
    }

    return model;
}

void ProblemModelSet::removeModel(const QString &name)
{
    QVector<ModelData>::iterator itr = d->data.begin();
    while (itr != d->data.end()) {
        if(itr->name == name)
            break;
        ++itr;
    }

    if(itr != d->data.end())
        d->data.erase(itr);

    emit removed(name);
}

QVector<ModelData> ProblemModelSet::models() const
{
    return d->data;
}

}

