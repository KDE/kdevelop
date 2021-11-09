/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROBLEMCONSTANTS_H
#define PROBLEMCONSTANTS_H

namespace KDevelop
{


// Which set of files should be tracked for errors.
enum ProblemScope {
    CurrentDocument,
    OpenDocuments,
    CurrentProject,
    AllProjects,
    DocumentsInPath,
    BypassScopeFilter // Special setting. Bypasses the filter in FilteredProblemStore
};

// How should problems be grouped
enum GroupingMethod
{
    NoGrouping,
    PathGrouping,
    SeverityGrouping
};

}

#endif

