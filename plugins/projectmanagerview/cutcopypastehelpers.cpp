/*
    SPDX-FileCopyrightText: 2017 Alexander Potashev <aspotashev@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "cutcopypastehelpers.h"

#include <QTreeWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QStyleOption>
#include <QPointer>
#include <QAbstractButton>

#include <KLocalizedString>
#include <KIO/DeleteJob>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <serialization/indexedstring.h>

using namespace KDevelop;

namespace CutCopyPasteHelpers
{

TaskInfo::TaskInfo(const TaskStatus status, const TaskType type,
                   const Path::List& src, const Path& dest)
    : m_status(status),
      m_type(type),
      m_src(src),
      m_dest(dest)
{
}

TaskInfo TaskInfo::createMove(const bool ok, const Path::List& src, const Path& dest)
{
    return TaskInfo(ok ? TaskStatus::SUCCESS : TaskStatus::FAILURE,
                    TaskType::MOVE, src, dest);
}

TaskInfo TaskInfo::createCopy(const bool ok, const Path::List& src, const Path& dest)
{
    return TaskInfo(ok ? TaskStatus::SUCCESS : TaskStatus::FAILURE,
                    TaskType::COPY, src, dest);
}

TaskInfo TaskInfo::createDeletion(const bool ok, const Path::List& src, const Path& dest)
{
    return TaskInfo(ok ? TaskStatus::SUCCESS : TaskStatus::FAILURE,
                    TaskType::DELETION, src, dest);
}

static QWidget* createPasteStatsWidget(QWidget *parent, const QVector<TaskInfo>& tasks)
{
    // TODO: Create a model for the task list, and use it here instead of using QTreeWidget
    auto* treeWidget = new QTreeWidget(parent);
    QList<QTreeWidgetItem *> items;
    items.reserve(tasks.size());
    for (const TaskInfo& task : tasks) {
        int srcCount = task.m_src.size();
        const bool withChildren = srcCount != 1;

        const QString destPath = task.m_dest.pathOrUrl();

        QString text;
        if (withChildren) {
            // Multiple source items in the current suboperation
            switch (task.m_type) {
                case TaskType::MOVE:
                    text = i18np("Move %1 item into %2", "Move %1 items into %2", srcCount, destPath);
                    break;
                case TaskType::COPY:
                    text = i18np("Copy %1 item into %2", "Copy %1 items into %2", srcCount, destPath);
                    break;
                case TaskType::DELETION:
                    text = i18np("Delete %1 item", "Delete %1 items", srcCount);
                    break;
            }
        } else {
            // One source item in the current suboperation
            const QString srcPath = task.m_src[0].pathOrUrl();

            switch (task.m_type) {
                case TaskType::MOVE:
                    text = i18n("Move item %1 into %2", srcPath, destPath);
                    break;
                case TaskType::COPY:
                    text = i18n("Copy item %1 into %2", srcPath, destPath);
                    break;
                case TaskType::DELETION:
                    text = i18n("Delete item %1", srcPath);
                    break;
            }
        }

        QString tooltip;
        QString iconName;
        switch (task.m_status) {
            case TaskStatus::SUCCESS:
                tooltip = i18nc("@info:tooltip", "Suboperation succeeded");
                iconName = QStringLiteral("dialog-ok");
                break;
            case TaskStatus::FAILURE:
                tooltip = i18nc("@info:tooltip", "Suboperation failed");
                iconName = QStringLiteral("dialog-error");
                break;
            case TaskStatus::SKIPPED:
                tooltip = i18nc("@info:tooltip", "Suboperation skipped to prevent data loss");
                iconName = QStringLiteral("dialog-warning");
                break;
        }

        auto* item = new QTreeWidgetItem;
        item->setText(0, text);
        item->setIcon(0, QIcon::fromTheme(iconName));
        item->setToolTip(0, tooltip);
        items.append(item);

        if (withChildren) {
            for (const Path& src : task.m_src) {
                auto* childItem = new QTreeWidgetItem;
                childItem->setText(0, src.pathOrUrl());
                item->addChild(childItem);
            }
        }
    }
    treeWidget->insertTopLevelItems(0, items);
    treeWidget->headerItem()->setHidden(true);

    return treeWidget;
}

SourceToDestinationMap mapSourceToDestination(const Path::List& sourcePaths, const Path& destinationPath)
{
    // For example you are moving the following items into /dest/
    //   * /tests/
    //   * /tests/abc.cpp
    // If you pass them as is, moveFilesAndFolders() will crash (see note:
    // "Do not attempt to move subitems along with their parents").
    // Thus we filter out subitems from "Path::List filteredPaths".
    //
    // /tests/abc.cpp will be implicitly moved to /dest/tests/abc.cpp, for
    // that reason we add "/dest/tests/abc.cpp" into "result.finalPaths" as well as
    // "/dest/tests".
    //
    // "result.finalPaths" will be used to highlight destination items after
    // copy/move.
    Path::List sortedPaths = sourcePaths;
    std::sort(sortedPaths.begin(), sortedPaths.end());

    SourceToDestinationMap result;
    for (const Path& path : sortedPaths) {
        if (!result.filteredPaths.isEmpty() && result.filteredPaths.back().isParentOf(path)) {
            // think: "/tests"
            const Path& previousPath = result.filteredPaths.back();
            // think: "/dest" + "/".relativePath("/tests/abc.cpp") = /dest/tests/abc.cpp
            result.finalPaths[previousPath].append(Path(destinationPath, previousPath.parent().relativePath(path)));
        } else {
            // think: "/tests"
            result.filteredPaths.append(path);
            // think: "/dest" + "tests" = "/dest/tests"
            result.finalPaths[path].append(Path(destinationPath, path.lastPathSegment()));
        }
    }

    return result;
}

struct ClassifiedPaths
{
    // Items originating from projects open in this KDevelop session
    QHash<IProject*, QList<KDevelop::ProjectBaseItem*>> itemsPerProject;
    // Items that do not belong to known projects
    Path::List alienSrcPaths;
};

static ClassifiedPaths classifyPaths(const Path::List& paths, KDevelop::ProjectModel* projectModel)
{
    ClassifiedPaths result;
    for (const Path& path : paths) {
        const QList<ProjectBaseItem*> items = projectModel->itemsForPath(IndexedString(path.path()));
        if (!items.empty()) {
            for (ProjectBaseItem* item : items) {
                IProject* project = item->project();
                auto itemsIt = result.itemsPerProject.find(project);
                if (itemsIt == result.itemsPerProject.end()) {
                    itemsIt = result.itemsPerProject.insert(project, QList<KDevelop::ProjectBaseItem*>());
                }

                itemsIt->append(item);
            }
        } else {
            result.alienSrcPaths.append(path);
        }
    }

    return result;
}

QVector<TaskInfo> copyMoveItems(const Path::List& paths, ProjectBaseItem* destItem, const Operation operation)
{
    KDevelop::ProjectModel* projectModel = KDevelop::ICore::self()->projectController()->projectModel();
    const ClassifiedPaths cl = classifyPaths(paths, projectModel);

    QVector<TaskInfo> tasks;

    IProject* destProject = destItem->project();
    IProjectFileManager* destProjectFileManager = destProject->projectFileManager();
    ProjectFolderItem* destFolder = destItem->folder();
    Path destPath = destFolder->path();
    const auto& srcProjects = cl.itemsPerProject.keys();
    for (IProject* srcProject : srcProjects) {
        auto itemsList = cl.itemsPerProject[srcProject];

        // make sure each item is only moved once
        std::sort(itemsList.begin(), itemsList.end(), [](auto const& a, auto const& b) {
            return a->path() < b->path();
        });
        itemsList.erase(std::unique(itemsList.begin(), itemsList.end(),
                                    [](auto const& a, auto const& b) {
                                        return a->path() == b->path();
                                    }),
                        itemsList.cend());

        Path::List pathsList;
        pathsList.reserve(itemsList.size());
        for (const auto* const item : std::as_const(itemsList)) {
            pathsList.append(item->path());
        }

        if (srcProject == destProject) {
            if (operation == Operation::CUT) {
                // Move inside project
                const bool ok = destProjectFileManager->moveFilesAndFolders(itemsList, destFolder);
                tasks.append(TaskInfo::createMove(ok, pathsList, destPath));
            } else {
                // Copy inside project
                const bool ok = destProjectFileManager->copyFilesAndFolders(pathsList, destFolder);
                tasks.append(TaskInfo::createCopy(ok, pathsList, destPath));
            }
        } else {
            // Copy/move between projects:
            //  1. Copy and add into destination project;
            //  2. Remove from source project.
            const bool copy_ok = destProjectFileManager->copyFilesAndFolders(pathsList, destFolder);
            tasks.append(TaskInfo::createCopy(copy_ok, pathsList, destPath));

            if (operation == Operation::CUT) {
                if (copy_ok) {
                    IProjectFileManager* srcProjectFileManager = srcProject->projectFileManager();
                    const bool deletion_ok = srcProjectFileManager->removeFilesAndFolders(itemsList);
                    tasks.append(TaskInfo::createDeletion(deletion_ok, pathsList, destPath));
                } else {
                    tasks.append(TaskInfo(TaskStatus::SKIPPED, TaskType::DELETION, pathsList, destPath));
                }
            }
        }
    }

    // Copy/move items from outside of all open projects
    if (!cl.alienSrcPaths.isEmpty()) {
        const bool alien_copy_ok = destProjectFileManager->copyFilesAndFolders(cl.alienSrcPaths, destFolder);
        tasks.append(TaskInfo::createCopy(alien_copy_ok, cl.alienSrcPaths, destPath));

        if (operation == Operation::CUT) {
            if (alien_copy_ok) {
                QList<QUrl> urlsToDelete;
                urlsToDelete.reserve(cl.alienSrcPaths.size());
                for (const Path& path : cl.alienSrcPaths) {
                    urlsToDelete.append(path.toUrl());
                }

                KIO::DeleteJob* deleteJob = KIO::del(urlsToDelete);
                const bool deletion_ok = deleteJob->exec();
                tasks.append(TaskInfo::createDeletion(deletion_ok, cl.alienSrcPaths, destPath));
            } else {
                tasks.append(TaskInfo(TaskStatus::SKIPPED, TaskType::DELETION, cl.alienSrcPaths, destPath));
            }
        }
    }

    return tasks;
}

void showWarningDialogForFailedPaste(QWidget* parent, const QVector<TaskInfo>& tasks)
{
    auto* dialog = new QDialog(parent);

    dialog->setWindowTitle(i18nc("@title:window", "Paste Failed"));

    auto *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    QObject::connect(buttonBox, &QDialogButtonBox::clicked, dialog, &QDialog::accept);

    dialog->setWindowModality(Qt::WindowModal);
    dialog->setModal(true);

    auto* mainWidget = new QWidget(dialog);
    auto* mainLayout = new QVBoxLayout(mainWidget);
    const int spacingHint = mainWidget->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
    mainLayout->setSpacing(spacingHint * 2); // provide extra spacing
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout, 0);

    auto* iconLabel = new QLabel(mainWidget);

    // Icon
    QStyleOption option;
    option.initFrom(mainWidget);
    QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
    iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));

    auto* iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);

    hLayout->addLayout(iconLayout, 0);
    hLayout->addSpacing(spacingHint);

    const QString text = i18n("Failed to paste. Below is a list of suboperations that have been attempted.");
    auto* messageLabel = new QLabel(text, mainWidget);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    hLayout->addWidget(messageLabel, 5);

    QWidget* statsWidget = createPasteStatsWidget(dialog, tasks);

    auto* topLayout = new QVBoxLayout;
    dialog->setLayout(topLayout);
    topLayout->addWidget(mainWidget);
    topLayout->addWidget(statsWidget, 1);
    topLayout->addWidget(buttonBox);

    dialog->setMinimumSize(300, qMax(150, qMax(iconLabel->sizeHint().height(), messageLabel->sizeHint().height())));

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

} // namespace CutCopyPasteHelpers
