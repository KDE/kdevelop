/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCLAZY_PLUGIN_H
#define KDEVCLAZY_PLUGIN_H

// KDevPlatform
#include <interfaces/iplugin.h>
// Qt
#include <QSharedPointer>

namespace Clazy
{

class ChecksDB;
class Analyzer;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const QVariantList& = QVariantList());
    ~Plugin() override;

public: // KDevelop::IPlugin API
    void unload() override;
    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

public:
    QSharedPointer<const ChecksDB> checksDB() const;
    QSharedPointer<const ChecksDB> loadedChecksDB();

private:
    void reloadDB();

private:
    Analyzer* m_analyzer;
    QSharedPointer<ChecksDB> m_db;
};

}

#endif
