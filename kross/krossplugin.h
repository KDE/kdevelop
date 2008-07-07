/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KROSSPROJECTMANAGER_H
#define KROSSPROJECTMANAGER_H

#include <iplugin.h>

#include <kross/core/action.h>

#include "krossbuildsystemmanager.h"

namespace Kross { class Action; }

class KrossPlugin : public KDevelop::IPlugin, public KrossBuildSystemManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IBuildSystemManager )
Q_INTERFACES( KDevelop::IProjectFileManager )
public:
    explicit KrossPlugin( QObject* parent = 0, const QVariantList& args = QVariantList() );
    virtual ~KrossPlugin() {}


private:
    Kross::Action* action;

    KrossBuildSystemManager* m_script;
};

#endif
