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

class CompletionSettings : public KDevelop::ICompletionSettings {
public:
    CompletionSettings() : m_level(AlwaysFull), m_automatic(true), m_enableSemanticHighlighting(true), m_localVariableColorizationLevel(120), m_highlightSemanticProblems(true) {
    }
    
    virtual CompletionLevel completionLevel() const {
        return m_level;
    }
        
    virtual bool automaticCompletionEnabled() const {
        return m_automatic;
    }
    
    void emitChanged() {
        emit settingsChanged(this);
    }
    
    virtual int localVariableColorizationLevel() const {
        return m_localVariableColorizationLevel;
    }
    
    virtual bool semanticHighlightingEnabled() const {
        return m_enableSemanticHighlighting;
    }
    
    virtual bool highlightSemanticProblems() const {
        return m_highlightSemanticProblems;
    }
    
    static CompletionSettings& self();
    
    CompletionLevel m_level;
    bool m_automatic, m_enableSemanticHighlighting, m_highlightSemanticProblems;
    int m_localVariableColorizationLevel;
};

#endif
