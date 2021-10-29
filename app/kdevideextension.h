/*
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVIDEEXTENSION_H
#define KDEVIDEEXTENSION_H

#include <shell/shellextension.h>


class KDevIDEExtension : public KDevelop::ShellExtension {
public:
    static void init();

    QString xmlFile() override;
    QString executableFilePath() override;
    KDevelop::AreaParams defaultArea() override;
    QString projectFileExtension() override;
    QString projectFileDescription() override;
    QStringList defaultPlugins() override;

protected:
    KDevIDEExtension();
};

#endif








