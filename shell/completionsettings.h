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

#ifndef COMPLETIONSETTINGS_H
#define COMPLETIONSETTINGS_H

#include <interfaces/icompletionsettings.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>  
#include <kglobal.h>

class CompletionSettings : public KDevelop::ICompletionSettings {
public:
    CompletionSettings() : m_level(AlwaysFull), m_automatic(true), m_enableSemanticHighlighting(true), m_highlightSemanticProblems(true), m_localVariableColorizationLevel(120) {
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
    
    virtual int localVariableColorizationLevel() const {
        return readIntConfig("localVariableColorization", m_localVariableColorizationLevel);
    }
    
    virtual bool semanticHighlightingEnabled() const {
        return readBoolConfig("enableSemanticHighlighting", m_enableSemanticHighlighting);
    }
    
    virtual bool highlightSemanticProblems() const {
        return readBoolConfig("highlightSemanticProblems", m_highlightSemanticProblems);
    }
    
    static CompletionSettings& self();
    
    CompletionLevel m_level;
    bool m_automatic, m_enableSemanticHighlighting, m_highlightSemanticProblems;
    int m_localVariableColorizationLevel;
};

#endif
