/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SOURCEFORMATTERPLUGIN_H
#define SOURCEFORMATTERPLUGIN_H

#include <interfaces/iplugin.h>

class SourceFormatterPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    /**
     * Constructor with arguments as needed with KPluginFactory
     */
    SourceFormatterPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~SourceFormatterPlugin() override;

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

private:
};

#endif // SOURCEFORMATTERPLUGIN_H
