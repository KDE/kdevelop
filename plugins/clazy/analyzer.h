/*
 * This file is part of KDevelop
 *
 * Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CLAZY_ANALYZER_H
#define CLAZY_ANALYZER_H

// CompileAnalyzer
#include <compileanalyzer.h>

namespace Clazy
{

class Plugin;

class Analyzer : public KDevelop::CompileAnalyzer
{
    Q_OBJECT

public:
    Analyzer(Plugin* plugin, QObject* parent);
    ~Analyzer();

protected:
    KDevelop::CompileAnalyzeJob* createJob(KDevelop::IProject* project, const KDevelop::Path& buildDirectory,
                                           const QUrl& url, const QStringList& filePaths) override;
    bool isOutputToolViewPreferred() const override;

private:
    Plugin* const m_plugin;
};

}

#endif
