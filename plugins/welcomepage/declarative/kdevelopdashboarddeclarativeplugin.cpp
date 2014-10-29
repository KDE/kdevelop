/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kdevelopdashboarddeclarativeplugin.h"
#include "icoreobject.h"
#include <qqml.h>
#include <vcs/models/brancheslistmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>

void KDevplatformDeclarativePlugin::registerTypes(const char* uri)
{
    qmlRegisterSingletonType<KDevelop::ICore>(uri, 1, 0, "ICore", [](QQmlEngine*, QJSEngine*) -> QObject* { return KDevelop::ICore::self(); });
    qmlRegisterType<KDevelop::BranchesListModel>(uri, 1, 0, "BranchesListModel");

    qmlRegisterType<KDevelop::IProject>();
}
