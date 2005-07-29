/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevbuildtool.h"

#include <kdevmakefrontend.h>
#include <kdevappfrontend.h>

KDevBuildTool::KDevBuildTool(const KDevPluginInfo* info, QObject* parent, const char* name)
    :KDevProject(info, parent, name)
{
}

KDevMakeFrontend * KDevBuildTool::makeFrontend()
{
    return extension<KDevMakeFrontend>("KDevelop/MakeFrontend");
}

KDevAppFrontend * KDevBuildTool::appFrontend()
{
    return extension<KDevAppFrontend>("KDevelop/AppFrontend");
}

