/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef BUILDERJOB_H
#define BUILDERJOB_H

#include <QtCore/QList>

#include <kcompositejob.h>

namespace KDevelop
{
class ProjectBaseItem;
class IProject;
}

class BuildItem;

class KConfigGroup;

class BuilderJob : public KCompositeJob
{
    Q_OBJECT
public:
    enum BuildType { Build, Prune, Configure, Install, Clean };
    BuilderJob( BuildType, const QList<KDevelop::ProjectBaseItem*>& );
    BuilderJob( BuildType, const QList<BuildItem>& );
    BuilderJob( BuildType, const QList<KDevelop::IProject*>& );
    void start();
private:
    void slotResult( KJob* );
    void addJob( BuildType, KDevelop::ProjectBaseItem* );
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
