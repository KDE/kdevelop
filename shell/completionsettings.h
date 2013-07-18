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

class CompletionSettings : public KDevelop::ICompletionSettings {
public:
    CompletionSettings()
        : m_level(MinimalWhenAutomatic), m_automatic(true),
          m_highlightSemanticProblems(true), m_highlightProblematicLines(false), m_showMultiLineInformation(false),
          m_localColorizationLevel(170), m_globalColorizationLevel(255), m_minFilesForSimplifiedParsing(100000)
    {
        m_todoMarkerWords << "TODO" << "FIXME";
    }

    bool readBoolConfig(QString name, bool _default = false) const {
        KConfigGroup group(KGlobal::config(), "Language Support");
        return group.readEntry( name, _default );
    }

    int readIntConfig(QString name, int _default = 0) const {
        KConfigGroup group(KGlobal::config(), "Language Support");
        return group.readEntry( name, _default );
    }
    
    virtual CompletionLevel completionLevel() const {
        CompletionLevel level(m_level);
        if(readBoolConfig("alwaysFullCompletion"))
            level = AlwaysFull;
        if(readBoolConfig("minimalAutomaticCompletion"))
            level = MinimalWhenAutomatic;
        if(readBoolConfig("alwaysMinimalCompletion"))
            level = Minimal;
        
        return level;
    }
        
    virtual bool automaticCompletionEnabled() const {
        return readBoolConfig("Automatic Invocation", m_automatic);
    }
    
    void emitChanged() {
        emit settingsChanged(this);
    }
    
    virtual int localColorizationLevel() const {
        return readIntConfig("localColorization", m_localColorizationLevel);
    }
    
    virtual int globalColorizationLevel() const {
        return readIntConfig("globalColorization", m_globalColorizationLevel);
    }
    
    virtual bool highlightSemanticProblems() const {
        return readBoolConfig("highlightSemanticProblems", m_highlightSemanticProblems);
    }
    
    virtual bool highlightProblematicLines() const {
        return readBoolConfig("highlightProblematicLines", m_highlightProblematicLines);
    }
    
    virtual bool showMultiLineSelectionInformation() const {
        return readBoolConfig("showMultiLineSelectionInformation", m_showMultiLineInformation);
    }
    
    virtual int minFilesForSimplifiedParsing() const {
        return readIntConfig("minFilesForSimplifiedParsing", m_minFilesForSimplifiedParsing);
    }

    virtual QStringList todoMarkerWords() const;
    
    static CompletionSettings& self();
    
    CompletionLevel m_level;
    bool m_automatic, m_highlightSemanticProblems, m_highlightProblematicLines, m_showMultiLineInformation;
    int m_localColorizationLevel;
    int m_globalColorizationLevel;
    int m_minFilesForSimplifiedParsing;
    QStringList m_todoMarkerWords;
};

#endif
