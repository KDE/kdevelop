/*  This file is part of KDevelop
    Copyright (C) 2017  Alexander Potashev <aspotashev@gmail.com>

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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_CUTCOPYPASTEHELPERS_H
#define KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_CUTCOPYPASTEHELPERS_H

#include <util/path.h>
#include <project/projectmodel.h>

namespace CutCopyPasteHelpers
{

enum class Operation {
    COPY,
    CUT,
};

enum class TaskStatus
{
    SUCCESS,
    FAILURE,
    SKIPPED,
};

enum class TaskType
{
    COPY,
    MOVE,
    DELETION,
};

struct TaskInfo
{
    TaskInfo() = default;
    TaskInfo(const TaskStatus status, const TaskType type,
             const KDevelop::Path::List& src, const KDevelop::Path& dest);

    static TaskInfo createMove(const bool ok, const KDevelop::Path::List& src, const KDevelop::Path& dest);
    static TaskInfo createCopy(const bool ok, const KDevelop::Path::List& src, const KDevelop::Path& dest);
    static TaskInfo createDeletion(const bool ok, const KDevelop::Path::List& src, const KDevelop::Path& dest);

    TaskStatus m_status;
    TaskType m_type;
    KDevelop::Path::List m_src;
    KDevelop::Path m_dest;
};

struct SourceToDestinationMap
{
    KDevelop::Path::List filteredPaths;

    // finalPaths is a map: source path -> new paths. If source path
    // succeeds to copy/move, then the new paths must be highlighted
    // in the project manager view.
    // Highlighting of all destination files without regard of which
    // operations were successful won't work in the case when the destination
    // file already exists and a replacing copy/move fails.
    QHash<KDevelop::Path, KDevelop::Path::List> finalPaths;
};

SourceToDestinationMap mapSourceToDestination(const KDevelop::Path::List& sourcePaths, const KDevelop::Path& destinationPath);

QVector<TaskInfo> copyMoveItems(const KDevelop::Path::List& paths, KDevelop::ProjectBaseItem* destItem, const Operation operation);

void showWarningDialogForFailedPaste(QWidget* parent, const QVector<TaskInfo>& tasks);

} // namespace CutCopyPasteHelpers

Q_DECLARE_TYPEINFO(CutCopyPasteHelpers::TaskInfo, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_CUTCOPYPASTEHELPERS_H
