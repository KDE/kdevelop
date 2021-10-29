/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTCONFIGPAGE_H
#define KDEVPLATFORM_PROJECTCONFIGPAGE_H


#include <interfaces/iproject.h>
#include <interfaces/configpage.h>

#include <util/path.h>

#include "projectconfigskeleton.h"

class KComponentData;
class QWidget;

namespace KDevelop {

/** This is needed because IProject does not expose these methods */
struct ProjectConfigOptions
{
    QString developerTempFile;
    Path developerFile;
    QString projectTempFile;
    KDevelop::IProject* project;
};

}

/**
 * @tparam T a class derived from KDevelop::ProjectConfigSkeleton.
 */
template<typename T>
class ProjectConfigPage : public KDevelop::ConfigPage
{
public:
    ProjectConfigPage(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
        : KDevelop::ConfigPage(plugin, initConfigSkeleton(options), parent)
        , m_project(options.project)
    {
        static_assert(std::is_base_of<KDevelop::ProjectConfigSkeleton, T>::value, "T must inherit from KDevelop::ProjectConfigSkeleton");
        KDevelop::ProjectConfigSkeleton* conf = T::self();
        conf->setDeveloperTempFile(options.developerTempFile);
        conf->setDeveloperFile(options.developerFile);
        conf->setProjectTempFile(options.projectTempFile);
        conf->setProjectFile(m_project->projectFile());
    }

    ~ProjectConfigPage() override
    {
        // we have to delete T::self otherwise we get the following message on the
        // next call to T::instance(QString):
        // "T::instance called after the first use - ignoring"
        // which means that we will continue using the old file
        delete T::self();
    }

    KDevelop::IProject* project() const
    {
        return m_project;
    }
private:
    static inline KDevelop::ProjectConfigSkeleton* initConfigSkeleton(const KDevelop::ProjectConfigOptions& options)
    {
        T::instance(options.developerTempFile);
        return T::self();
    }
private:
    KDevelop::IProject* m_project;
};

#endif

