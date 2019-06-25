/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_PROJECTCONFIGSKELETON_H
#define KDEVPLATFORM_PROJECTCONFIGSKELETON_H

#include "projectexport.h"

#include <KConfigSkeleton>

namespace KDevelop
{

class Path;
class ProjectConfigSkeletonPrivate;

class KDEVPLATFORMPROJECT_EXPORT ProjectConfigSkeleton: public KConfigSkeleton
{
    Q_OBJECT

public:
    ~ProjectConfigSkeleton() override;

    void setDeveloperTempFile( const QString& );
    void setProjectTempFile( const QString& );
    void setProjectFile( const Path& );
    void setDeveloperFile( const Path& );
    void setDefaults() override;
    bool useDefaults( bool b ) override;
    bool writeConfig();

    Path projectFile() const;
    Path developerFile() const;

protected:
    explicit ProjectConfigSkeleton( KSharedConfigPtr config );
    /**
     * Constructs a new skeleton, the skeleton will write to the developer
     * configuration file, which is by default located in projectdir/.kdev4
     * The defaults will be set from the project file, which is in the projectdir
     *
     * @param configname The absolute filename of the developer configuration file
     */
    explicit ProjectConfigSkeleton( const QString & configname );

private:
    const QScopedPointer<class ProjectConfigSkeletonPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectConfigSkeleton)
};

}
#endif

