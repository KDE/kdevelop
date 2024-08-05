/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <interfaces/iplugin.h>


namespace KDevelop {
class Declaration;
}

class OutlineViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit OutlineViewPlugin(QObject* parent, const KPluginMetaData& metaData,
                               const QVariantList& args = QVariantList());
    ~OutlineViewPlugin() override;

public: // KDevelop::Plugin overrides
    void unload() override;

private:
    class OutlineViewFactory* m_factory;
    Q_DISABLE_COPY(OutlineViewPlugin)
};
