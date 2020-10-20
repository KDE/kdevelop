/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "documentswitcheritem.h"
#include "documentswitchertreeview.h"

#include <KFileItem>

#include <QDir>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

DocumentSwitcherItem::DocumentSwitcherItem(KDevelop::IDocument *document)
{
    const QUrl &url = document->url();

    KDevelop::IProjectController *projectController = KDevelop::ICore::self()->projectController();

    // Find file icon : file type icon or document unsaved icon.
    switch (document->state())
    {
        case KDevelop::IDocument::Clean:
        {
            setIcon(QIcon::fromTheme(KFileItem(url, QString(), 0).iconName()));
            break;
        }
        case KDevelop::IDocument::Modified:
        {
            setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
            break;
        }
        case KDevelop::IDocument::Dirty:
        {
            setIcon(QIcon::fromTheme(QStringLiteral("document-revert")));
            break;
        }
        case KDevelop::IDocument::DirtyAndModified:
        {
            setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
            break;
        }
    }

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

