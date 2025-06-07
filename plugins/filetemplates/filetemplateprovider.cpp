/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filetemplateprovider.h"

#include <KLocalizedString>

#include <QIcon>

using namespace KDevelop;

QString FileTemplateProvider::name() const
{
    return i18n("File Templates");
}

QIcon FileTemplateProvider::icon() const
{
    return QIcon::fromTheme(QStringLiteral("code-class"));
}

QString FileTemplateProvider::modelTypePrefix() const
{
    return QStringLiteral("kdevfiletemplates");
}

QString FileTemplateProvider::knsConfigurationFile() const
{
    return QStringLiteral("kdevfiletemplates.knsrc");
}
