/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKEPROJECTSCOPE_H
#define QMAKEPROJECTSCOPE_H

#include <kurl.h>

template <typename T> class QList;

namespace QMake{
    class ProjectAST;
}

class QMakeProjectScope
{
    public:

        static const QStringList FileVariables;

        QMakeProjectScope( const KUrl& projectfile );
        ~QMakeProjectScope();

        QList<QMakeProjectScope*> subProjects() const;

        QList<KUrl> files() const;

        KUrl absoluteDirUrl() const;

    private:
        QMake::ProjectAST* m_ast;
        KUrl m_projectFileUrl;
};

#endif

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
