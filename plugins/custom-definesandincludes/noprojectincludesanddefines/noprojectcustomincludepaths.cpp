/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "noprojectcustomincludepaths.h"

#include "ui_noprojectcustomincludepaths.h"

#include <KLocalizedString>

#include <QFileDialog>
#include <QUrl>


NoProjectCustomIncludePaths::NoProjectCustomIncludePaths(QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui::CustomIncludePaths)
{
    m_ui->setupUi(this);
    m_ui->storageDirectory->setMode(KFile::Directory);

    setWindowTitle(i18nc("@title:window", "Setup Custom Include Paths"));

    connect(m_ui->directorySelector, &QPushButton::clicked, this, &NoProjectCustomIncludePaths::openAddIncludeDirectoryDialog);
}

void NoProjectCustomIncludePaths::setStorageDirectory(const QString& path)
{
    m_ui->storageDirectory->setUrl(QUrl::fromLocalFile(path));
}

QString NoProjectCustomIncludePaths::storageDirectory() const
{
    return m_ui->storageDirectory->url().toLocalFile();
}

void NoProjectCustomIncludePaths::appendCustomIncludePath(const QString& path)
{
    m_ui->customIncludePaths->appendPlainText(path);
}

QString NoProjectCustomIncludePaths::customIncludePaths() const
{
    return m_ui->customIncludePaths->toPlainText();
}

void NoProjectCustomIncludePaths::setCustomIncludePaths(const QString& paths)
{
    m_ui->customIncludePaths->setPlainText(paths);
}

void NoProjectCustomIncludePaths::openAddIncludeDirectoryDialog()
{
    const QString dirName = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Select Directory to Include"));
    if (dirName.isEmpty())
        return;

    appendCustomIncludePath(dirName);
}

#include "moc_noprojectcustomincludepaths.cpp"
