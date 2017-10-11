/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MESONBUILDER_H
#define MESONBUILDER_H

#include <project/interfaces/iprojectbuilder.h>

class MesonBuilder : public QObject, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)
public:
    explicit MesonBuilder(QObject* parent);

    KJob* build(KDevelop::ProjectBaseItem* item) override;
    KJob* clean(KDevelop::ProjectBaseItem* item) override;
    KJob* install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath) override;

    KJob* configure(KDevelop::IProject* project) override;

private:
    KDevelop::IProjectBuilder* m_ninjaBuilder = nullptr;
};

#endif // MESONBUILDER_H
