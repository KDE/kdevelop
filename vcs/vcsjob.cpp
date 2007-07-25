/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "vcsjob.h"

namespace KDevelop
{
    class VcsJobPrivate
    {
        public:
            VcsJob::JobType m_type;
    };

    VcsJob::VcsJob( QObject* parent )
        : KJob(parent), d(new VcsJobPrivate)
    {
    }

    VcsJob::~VcsJob()
    {
        delete d;
    }

    VcsJob::JobType VcsJob::type()
    {
        return d->m_type;
    }

    void VcsJob::setType( VcsJob::JobType t )
    {
        d->m_type = t;
    }
}

#include "vcsjob.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
