/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTFOLDER_H
#define KDEVPLATFORM_PROJECTFOLDER_H

#include "documentclassesfolder.h"

namespace KDevelop {
class IProject;
}

namespace ClassModelNodes {
/// Special folder
/// It displays all items of a given project.
class ProjectFolder
    : public DocumentClassesFolder
{
    Q_OBJECT

public:
    ProjectFolder(NodesModelInterface* a_model, KDevelop::IProject* project);
    explicit ProjectFolder(NodesModelInterface* a_model);

    void populateNode() override;

protected:
    KDevelop::IProject* m_project;
};

/// Filter for the project folder.
/// TODO: can't we share some code with FilteredAllClassesFolder somehow?
class FilteredProjectFolder
    : public ProjectFolder
{
    Q_OBJECT

public:
    FilteredProjectFolder(NodesModelInterface* a_model, KDevelop::IProject* project);

public: // Operations.
    /// Call this to update the classes filter string.
    void updateFilterString(const QString& a_newFilterString);

private: // DocumentClassesFolder overrides
    bool isClassFiltered(const KDevelop::QualifiedIdentifier& a_id) override;

private:
    /// We'll use this string to display only classes that match this string.
    QString m_filterString;
};
}

#endif // KDEVPLATFORM_PROJECTFOLDER_H
