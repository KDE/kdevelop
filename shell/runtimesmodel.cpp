/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "runtimesmodel.h"

KDevelop::RuntimesModel::RuntimesModel(KDevelop::RuntimeController* controller)
    : QAbstractListModel(controller)
    , m_controller(controller)
{
}

KDevelop::IRuntime * KDevelop::RuntimesModel::runtimeAt(int row) const
{
    return m_controller->availableRuntimes().at(row);
}

int KDevelop::RuntimesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_controller->availableRuntimes().count();
}

QVariant KDevelop::RuntimesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.parent().isValid() || index.row() >= m_controller->availableRuntimes().count())
        return {};

    const auto runtime = runtimeAt(index.row());
    switch(role) {
        case Qt::DisplayRole:
            return runtime->name();
        case Qt::UserRole:
            return QVariant::fromValue<QObject*>(runtime);
    }
    return {};
}
