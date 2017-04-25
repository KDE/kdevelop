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

#include "runtimecontroller.h"

namespace KDevelop
{

class RuntimeController;

class RuntimesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    friend class KDevelop::RuntimeController;

    RuntimesModel(RuntimeController* controller);

    QVariant data(const QModelIndex & index, int role) const override;
    int rowCount(const QModelIndex & parent) const override;
    KDevelop::IRuntime* runtimeAt(int row) const;

private:
    KDevelop::RuntimeController* const m_controller;
};

}
