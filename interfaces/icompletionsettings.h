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

#ifndef KDEVPLATFORM_ICOMPLETIONSETTINGS_H
#define KDEVPLATFORM_ICOMPLETIONSETTINGS_H

#include <QtCore/QObject>
#include "interfacesexport.h"

namespace KDevelop {
    class KDEVPLATFORMINTERFACES_EXPORT ICompletionSettings : public QObject {
        Q_OBJECT
        public:
            virtual ~ICompletionSettings();
            enum CompletionLevel {
                Minimal,
                MinimalWhenAutomatic,
                AlwaysFull,
                LAST_LEVEL
            };

            Q_SCRIPTABLE virtual int minFilesForSimplifiedParsing() const = 0;
            
            Q_SCRIPTABLE virtual CompletionLevel completionLevel() const = 0;
            
            Q_SCRIPTABLE virtual bool automaticCompletionEnabled() const = 0;
            
            Q_SCRIPTABLE virtual int localColorizationLevel() const = 0;
            
            Q_SCRIPTABLE virtual int globalColorizationLevel() const = 0;
            
            Q_SCRIPTABLE virtual bool highlightSemanticProblems() const = 0;
            
            Q_SCRIPTABLE virtual bool highlightProblematicLines() const = 0;
            
            Q_SCRIPTABLE virtual bool showMultiLineSelectionInformation() const = 0;
            
            Q_SCRIPTABLE virtual QStringList todoMarkerWords() const = 0;
            
        Q_SIGNALS:
            void settingsChanged(ICompletionSettings*);
    };
}

#endif
