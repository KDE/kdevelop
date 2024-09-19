/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documentswitcheritem.h"
#include "documentswitchertreeview.h"

#include <QDir>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

DocumentSwitcherItem::DocumentSwitcherItem(KDevelop::IDocument *document)
{
    setIcon(document->icon());

    const QUrl &url = document->url();

    KDevelop::IProjectController *projectController = KDevelop::ICore::self()->projectController();

    // Extract file name and path.
    QString text = url.fileName();
    QString path = projectController->prettyFilePath(url, KDevelop::IProjectController::FormatPlain);

    const bool isPartOfOpenProject = QDir::isRelativePath(path);
    if (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    if (isPartOfOpenProject) {
        const int projectNameSize = path.indexOf(QLatin1Char(':'));

        // first: project name, second: path to file in project (might be just '/' when the file is in the project root dir)
        const QPair<QString, QString> fileInProjectInfo = (projectNameSize < 0)
            ? qMakePair(path, QStringLiteral("/"))
            : qMakePair(path.left(projectNameSize), path.mid(projectNameSize + 1));

        text = QStringLiteral("%1 (%2:%3)").arg(text, fileInProjectInfo.first, fileInProjectInfo.second);
    }
    else {
        text += QLatin1String(" (") + path + QLatin1Char(')');
    }

    setText(text);

    // Set item data.
    KDevelop::IProject *project = projectController->findProjectForUrl(url);
    setData(QVariant::fromValue(project), DocumentSwitcherTreeView::ProjectRole);
}

DocumentSwitcherItem::~DocumentSwitcherItem() = default;

