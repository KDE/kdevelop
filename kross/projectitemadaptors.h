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

#ifndef PROJECTITEMADAPTORS_H
#define PROJECTITEMADAPTORS_H

#include "projectmodel.h"

class IProject;

namespace KDevelop
{
        
    class ProjectBaseItemAdaptor : public QObject
    {
        Q_OBJECT
        protected:
            ProjectBaseItemAdaptor(ProjectBaseItem* item) : QObject(), m_item(item) {}
            
        public slots:
            IProject* project() const { return m_item->project(); }
            
        private:
            ProjectBaseItem* m_item;
    };
    
    class ProjectFolderItemAdaptor : public ProjectBaseItemAdaptor
    {
        Q_OBJECT
        public:
            ProjectFolderItemAdaptor(ProjectFolderItem* item) : ProjectBaseItemAdaptor(item), m_item(item) {}
        public slots:
            KUrl url() const { return m_item->url(); }
            bool isProjectRoot() const { return m_item->isProjectRoot(); }
            bool hasFileOrFolder(const QString& name) const { return m_item->hasFileOrFolder(name); }
            
        private:
            ProjectFolderItem *m_item;
    };
    
    class ProjectTargetItemAdaptor : public ProjectBaseItemAdaptor
    {
        Q_OBJECT
        public:
            ProjectTargetItemAdaptor(ProjectTargetItem* item) : ProjectBaseItemAdaptor(item) {}
        
    };
    
    class ProjectFileItemAdaptor : public ProjectBaseItemAdaptor
    {
        Q_OBJECT
        public:
            ProjectFileItemAdaptor(ProjectFileItem* item) : ProjectBaseItemAdaptor(item), m_item(item) {}
        public slots:
            KUrl url() const { return m_item->url(); }
            
        private:
            ProjectFileItem *m_item;
    };
}

#endif