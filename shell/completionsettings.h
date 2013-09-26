/***************************************************************************
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
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

#ifndef KDEVPLATFORM_COMPLETIONSETTINGS_H
#define KDEVPLATFORM_COMPLETIONSETTINGS_H

#include <interfaces/icompletionsettings.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>  
#include <kglobal.h>

namespace KDevelop
{

class CompletionSettings : public KDevelop::ICompletionSettings
{
public:
    virtual CompletionLevel completionLevel() const;

    virtual bool automaticCompletionEnabled() const;

    void emitChanged() { emit settingsChanged(this); }

    virtual int localColorizationLevel() const;

    virtual int globalColorizationLevel() const;

    virtual bool highlightSemanticProblems() const;

    virtual bool highlightProblematicLines() const;

    virtual bool showMultiLineSelectionInformation() const;

    virtual int minFilesForSimplifiedParsing() const;

    virtual QStringList todoMarkerWords() const;

    static CompletionSettings& self();

private:
    CompletionSettings();

    const CompletionLevel m_level;
    const bool m_automatic, m_highlightSemanticProblems, m_highlightProblematicLines, m_showMultiLineInformation;
    const int m_localColorizationLevel;
    const int m_globalColorizationLevel;
    const int m_minFilesForSimplifiedParsing;
    const QString m_todoMarkerWords;

    const KConfigGroup m_languageGroup;
};
}
#endif
