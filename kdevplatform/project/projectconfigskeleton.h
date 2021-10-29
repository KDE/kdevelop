/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

