/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "customincludepaths.h"

#include <kfiledialog.h>
#include <klocalizedstring.h>

CustomIncludePaths::CustomIncludePaths(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.storageDirectory->setMode(KFile::Directory);
    ui.sourceDirectory->setMode(KFile::Directory);
    ui.buildDirectory->setMode(KFile::Directory);

    setWindowTitle(i18n("Setup Custom Include Paths"));

    connect(ui.directorySelector, SIGNAL(clicked()), this, SLOT(openAddIncludeDirectoryDialog()));
}

void CustomIncludePaths::setStorageDirectoryUrl(const KUrl& url)
{
    ui.storageDirectory->setUrl(url);
}

KUrl CustomIncludePaths::storageDirectoryUrl() const
{
    return ui.storageDirectory->url();
}

void CustomIncludePaths::setSourceDirectoryUrl(const KUrl& url)
{
    ui.sourceDirectory->setUrl(url);
}

KUrl CustomIncludePaths::sourceDirectoryUrl() const
{
    return ui.sourceDirectory->url();
}

void CustomIncludePaths::setBuildDirectoryUrl(const KUrl& url)
{
    ui.buildDirectory->setUrl(url);
}

KUrl CustomIncludePaths::buildDirectoryUrl() const
{
    return ui.buildDirectory->url();
}

void CustomIncludePaths::appendCustomIncludePath(const QString& path)
{
    ui.customIncludePaths->appendPlainText(path);
}

QStringList CustomIncludePaths::customIncludePaths() const
{
    const QString pathsText = ui.customIncludePaths->document()->toPlainText();
    const QStringList paths = pathsText.split('\n', QString::SkipEmptyParts);
    return paths;
}

void CustomIncludePaths::setCustomIncludePaths(const QStringList& paths)
{
    ui.customIncludePaths->setPlainText(paths.join("\n"));
}

void CustomIncludePaths::openAddIncludeDirectoryDialog()
{
    const QString dirName = KFileDialog::getExistingDirectory(KUrl(), this, tr("Select directory to include"));
    if (dirName.isEmpty())
        return;

    appendCustomIncludePath(dirName);
}

#include "customincludepaths.moc"
