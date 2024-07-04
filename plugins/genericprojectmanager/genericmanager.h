/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2004, 2005 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GENERICMANAGER_H
#define KDEVPLATFORM_PLUGIN_GENERICMANAGER_H

#include <project/abstractfilemanagerplugin.h>

class GenericProjectManager: public KDevelop::AbstractFileManagerPlugin
{
    Q_OBJECT

public:
    explicit GenericProjectManager(QObject* parent, const KPluginMetaData& metaData,
                                   const QVariantList& args = QVariantList());
};

#endif // KDEVPLATFORM_PLUGIN_GENERICIMPORTER_H
