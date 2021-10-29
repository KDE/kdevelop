/*
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevideextension.h"

#include <QStringList>
#include <KLocalizedString>

#include <QApplication>

KDevIDEExtension::KDevIDEExtension()
 : ShellExtension()
{
}

void KDevIDEExtension::init()
{
    s_instance = new KDevIDEExtension();
}

QString KDevIDEExtension::xmlFile()
{
    return QStringLiteral("kdevelopui.rc");
}

QString KDevIDEExtension::executableFilePath()
{
    return QApplication::applicationDirPath() + QLatin1String("/kdevelop");
}

KDevelop::AreaParams KDevIDEExtension::defaultArea()
{
    KDevelop::AreaParams params = {QStringLiteral("code"), i18n("Code")};
    return params;
}

QString KDevIDEExtension::projectFileExtension()
{
    return QStringLiteral("kdev4");
}

QString KDevIDEExtension::projectFileDescription()
{
    return i18n("KDevelop Project Files");
}

QStringList KDevIDEExtension::defaultPlugins()
{
    return QStringList();
}

