/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    BypassScopeFilter    // Special setting. Bypasses the filter in FilteredProblemStore
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

