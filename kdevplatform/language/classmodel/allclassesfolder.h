/*
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ALLCLASSESFOLDER_H
#define KDEVPLATFORM_ALLCLASSESFOLDER_H

#include "documentclassesfolder.h"

namespace KDevelop {
class IProject;
}

namespace ClassModelNodes {
/// Special folder.
/// It displays all the classes in the projects by using the IProject
class AllClassesFolder
    : public DocumentClassesFolder
{
    Q_OBJECT

public:
    explicit AllClassesFolder(NodesModelInterface* a_model);

public: // Node overrides
    void nodeCleared() override;
    void populateNode() override;

private Q_SLOTS:
    // Project watching
    void projectOpened(KDevelop::IProject* project);
    void projectClosing(KDevelop::IProject* project);
};

/// Contains a filter for the all classes folder.
class FilteredAllClassesFolder
    : public AllClassesFolder
{
    Q_OBJECT

public:
    explicit FilteredAllClassesFolder(NodesModelInterface* a_model);

public: // Operations.
    /// Call this to update the classes filter string.
    void updateFilterString(const QString& a_newFilterString);

private: // DocumentClassesFolder overrides
    bool isClassFiltered(const KDevelop::QualifiedIdentifier& a_id) override;

private:
    /// We'll use this string to display only classes that match this string.
    QString m_filterString;
};
} // namespace ClassModelNodes

#endif
