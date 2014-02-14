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

#ifndef CUSTOMINCLUDEPATHS_H
#define CUSTOMINCLUDEPATHS_H

#include "ui_custom_include_paths.h"

#include <QWidget>

class KUrl;

class CustomIncludePaths : public QWidget
{
    Q_OBJECT

public:
    CustomIncludePaths(QWidget* parent = 0);

    void setStorageDirectoryUrl(const KUrl& url);
    KUrl storageDirectoryUrl() const;

    void setSourceDirectoryUrl(const KUrl& url);
    KUrl sourceDirectoryUrl() const;

    void setBuildDirectoryUrl(const KUrl& url);
    KUrl buildDirectoryUrl() const;

    void appendCustomIncludePath(const QString& path);
    QStringList customIncludePaths() const;
    void setCustomIncludePaths(const QStringList& paths);

private:
    Ui::CustomIncludePaths ui;

private Q_SLOTS:
    void openAddIncludeDirectoryDialog();
};

#endif
