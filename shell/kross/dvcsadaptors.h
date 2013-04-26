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

#ifndef KDEVPLATFORM_DVCSADAPTORS_H
#define KDEVPLATFORM_DVCSADAPTORS_H

#include <QObject>
#include <vcs/vcsrevision.h>

namespace KDevelop
{
    class VcsRevision;
    class VcsLocation;
    
    class VcsRevisionAdaptor : public QObject
    {
        Q_OBJECT
        public:
            VcsRevisionAdaptor(const VcsRevision& rev, QObject* parent) : QObject(parent), m_rev(rev) {}
        
        private:
            KDevelop::VcsRevision m_rev;
    };
    
    class VcsLocationAdaptor : public QObject
    {
        Q_OBJECT
        public:
            VcsLocationAdaptor(const VcsLocation* rev, QObject* parent) : QObject(parent), m_rev(rev) {}
        
        private:
            const KDevelop::VcsLocation* m_rev;
    };
}

#endif
