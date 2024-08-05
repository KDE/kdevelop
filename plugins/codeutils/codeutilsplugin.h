/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H

#include <interfaces/iplugin.h>

#include <QVariant>

class CodeUtilsPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit CodeUtilsPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~CodeUtilsPlugin() override;

private Q_SLOTS:
    void documentDeclaration();
};


#endif // KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H
