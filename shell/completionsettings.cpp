/***************************************************************************
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *   Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "completionsettings.h"

#include <KShell>

namespace KDevelop
{

static QString completionLevelToString(ICompletionSettings::CompletionLevel l)
{
    if (l < 0 || l >= ICompletionSettings::LAST_LEVEL) {
        return QString();
    }

    const static QString levels[ICompletionSettings::LAST_LEVEL] = {"Minimal", "MinimalWhenAutomatic", "AlwaysFull"};
    return levels[l];
}

CompletionSettings& CompletionSettings::self()
{
    static CompletionSettings settings;
    return settings;
}

QStringList CompletionSettings::todoMarkerWords() const
{
    const QString markers = m_languageGroup.readEntry("todoMarkerWords", m_todoMarkerWords);
    return KShell::splitArgs(markers);
}

int CompletionSettings::minFilesForSimplifiedParsing() const
{
    return m_languageGroup.readEntry("minFilesForSimplifiedParsing", m_minFilesForSimplifiedParsing);
}

bool CompletionSettings::showMultiLineSelectionInformation() const
{
    return m_languageGroup.readEntry("showMultiLineSelectionInformation", m_showMultiLineInformation);
}

bool CompletionSettings::highlightProblematicLines() const
{
    return m_languageGroup.readEntry("highlightProblematicLines", m_highlightProblematicLines);
}

bool CompletionSettings::highlightSemanticProblems() const
{
    return m_languageGroup.readEntry("highlightSemanticProblems", m_highlightSemanticProblems);
}

int CompletionSettings::globalColorizationLevel() const
{
    return m_languageGroup.readEntry("globalColorization", m_globalColorizationLevel);
}

int CompletionSettings::localColorizationLevel() const
{
    return m_languageGroup.readEntry("localColorization", m_localColorizationLevel);
}

bool CompletionSettings::automaticCompletionEnabled() const
{
    return m_languageGroup.readEntry("Automatic Invocation", m_automatic);
}

ICompletionSettings::CompletionLevel CompletionSettings::completionLevel() const
{
    const QString level = m_languageGroup.readEntry("completionDetail", completionLevelToString(m_level));

    for (int i = 0; i < ICompletionSettings::LAST_LEVEL; i++) {
        if (completionLevelToString(static_cast<CompletionLevel>(i)) == level) {
            return static_cast<CompletionLevel>(i);
        }
    }
    return m_level;
}

CompletionSettings::CompletionSettings()
    : m_level(MinimalWhenAutomatic), m_automatic(true),
      m_highlightSemanticProblems(true), m_highlightProblematicLines(false), m_showMultiLineInformation(false),
      m_localColorizationLevel(170), m_globalColorizationLevel(255), m_minFilesForSimplifiedParsing(100000),
      m_todoMarkerWords("TODO FIXME"),
      m_languageGroup(KGlobal::config(), "Language Support"){}
}
