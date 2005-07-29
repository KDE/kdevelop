/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef __qeditor_settings_h
#define __qeditor_settings_h

#include <qstring.h>

class KConfig;

class QEditorSettings
{
protected:
    QEditorSettings( KConfig* config );
    void init();
    
public:
    ~QEditorSettings();
    
    static QEditorSettings* self();

    KConfig* config() { return m_config; }
    const KConfig* config() const { return m_config; }

    void readConfig() { init(); }

    // groups
    QString generalGroup() const { return QString::fromLatin1("General"); }

    // settings
    bool wordWrap() const { return m_wordWrap; }
    void setWordWrap( bool enable );

    int tabStop() const { return m_tabStop; }
    void setTabStop( int tabStop );

    bool completeWordWithSpace() const { return m_completeWordWithSpace; }
    void setCompleteWordWithSpace( bool enable );

    bool parenthesesMatching() const { return m_parenthesesMatching; }
    void setParenthesesMatching( bool enable );

    bool showMarkers() const { return m_showMarkers; }
    void setShowMarkers( bool enable );

    bool showLineNumber() const { return m_showLineNumber; }
    void setShowLineNumber( bool enable );

    bool showCodeFoldingMarkers() const { return m_showCodeFoldingMarkers; }
    void setShowCodeFoldingMarkers( bool enable );

private:
    static QEditorSettings* m_self;
    KConfig* m_config;
    bool m_wordWrap;
    int m_tabStop;
    bool m_completeWordWithSpace;
    bool m_parenthesesMatching;
    bool m_showMarkers;
    bool m_showLineNumber;
    bool m_showCodeFoldingMarkers;

private:
    QEditorSettings( const QEditorSettings& );
    void operator = ( const QEditorSettings& );
};


#endif // __qeditor_settings_h
