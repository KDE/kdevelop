/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOPROJECTCUSTOMINCLUDEPATHS_H
#define NOPROJECTCUSTOMINCLUDEPATHS_H

#include <QDialog>

namespace Ui
{
class CustomIncludePaths;
}

class NoProjectCustomIncludePaths : public QDialog

{
    Q_OBJECT

public:
    explicit NoProjectCustomIncludePaths( QWidget* parent = nullptr );

    void setStorageDirectory( const QString& path );
    QString storageDirectory() const;

    void appendCustomIncludePath( const QString& path );
    QString customIncludePaths() const;
    void setCustomIncludePaths(const QString& paths);

private:
    Ui::CustomIncludePaths* m_ui;

private Q_SLOTS:
    void openAddIncludeDirectoryDialog();
};

#endif
