/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <KParts/MainWindow>

#include <QStandardPaths>

namespace Heaptrack
{
    QString findExecutable(const QString& fallbackExecutablePath)
    {
        QString executablePath = QStandardPaths::findExecutable(fallbackExecutablePath);
        return executablePath.isEmpty() ? fallbackExecutablePath : executablePath;
    }

    QWidget* activeMainWindow()
    {
        return KDevelop::ICore::self()->uiController()->activeMainWindow();
    }
}
