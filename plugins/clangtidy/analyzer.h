/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_ANALYZER_H
#define CLANGTIDY_ANALYZER_H

// CompileAnalyzer
#include <compileanalyzer.h>

namespace ClangTidy
{

class Plugin;
class CheckSetSelectionManager;

class Analyzer : public KDevelop::CompileAnalyzer
{
    Q_OBJECT

public:
    Analyzer(Plugin* plugin, CheckSetSelectionManager* checkSetSelectionManager, QObject* parent);
    ~Analyzer();

protected:
    KDevelop::CompileAnalyzeJob* createJob(KDevelop::IProject* project, const KDevelop::Path& buildDirectory,
                                           const QUrl& url, const QStringList& filePaths) override;

private:
    Plugin* const m_plugin;
    CheckSetSelectionManager* const m_checkSetSelectionManager;
};

}

#endif
