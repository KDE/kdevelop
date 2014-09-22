/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef KDEVPLATFORM_PROJECTCONFIGPAGE_H
#define KDEVPLATFORM_PROJECTCONFIGPAGE_H


#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/configpage.h>

#include <util/path.h>

#include "projectconfigskeleton.h"

class KComponentData;
class QWidget;
class QStringList;

namespace KDevelop {

/** This is needed because IProject does not expose these methods */
struct ProjectConfigOptions {
    QString developerTempFile;
    Path developerFile;
    QString projectTempFile;
    Path projectFile;
    QString projectName;
};

}

/**
 * @tparam T a class derived from KDevelop::ProjectConfigSkeleton.
 */
template<typename T>
class ProjectConfigPage : public KDevelop::ConfigPage
{
    static_assert(std::is_base_of<KDevelop::ProjectConfigSkeleton, T>::value, "T must inherit from KDevelop::ProjectConfigSkeleton");
public:
    ProjectConfigPage(const KDevelop::ProjectConfigOptions& options, QWidget* parent)
        : KDevelop::ConfigPage(initConfigSkeleton(options), parent), projectName(options.projectName)
    {
        KDevelop::ProjectConfigSkeleton* conf = T::self();
        conf->setDeveloperTempFile(options.developerTempFile);
        conf->setDeveloperFile(options.developerFile);
        conf->setProjectTempFile(options.projectTempFile);
        conf->setProjectFile(options.projectFile);
    }

    virtual ~ProjectConfigPage() {}

    KDevelop::IProject* project() const
    {
        return KDevelop::ICore::self()->projectController()->findProjectByName( projectName );
    }
private:
    static inline KDevelop::ProjectConfigSkeleton* initConfigSkeleton(const KDevelop::ProjectConfigOptions& options)
    {
        T::instance(options.developerTempFile);
        return T::self();
    }
private:
    QString projectName;
};

#endif

