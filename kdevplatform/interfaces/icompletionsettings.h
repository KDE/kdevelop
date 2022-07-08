/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ICOMPLETIONSETTINGS_H
#define KDEVPLATFORM_ICOMPLETIONSETTINGS_H

#include <QObject>
#include "interfacesexport.h"

namespace KDevelop {

class KDEVPLATFORMINTERFACES_EXPORT ICompletionSettings : public QObject
{
    Q_OBJECT

public:
    ~ICompletionSettings() override;

    enum CompletionLevel {
        Minimal,
        MinimalWhenAutomatic,
        AlwaysFull,
        LAST_LEVEL
    };

    enum ProblemInlineNotesLevel {
        NoProblemsInlineNotesLevel,
        ErrorsProblemInlineNotesLevel,
        WarningsAndErrorsProblemInlineNotesLevel,
        AllProblemsInlineNotesLevel
    };

    virtual int minFilesForSimplifiedParsing() const = 0;

    virtual CompletionLevel completionLevel() const = 0;

    virtual bool automaticCompletionEnabled() const = 0;

    virtual int localColorizationLevel() const = 0;
    virtual int globalColorizationLevel() const = 0;

    virtual bool highlightSemanticProblems() const = 0;
    virtual bool highlightProblematicLines() const = 0;
    virtual ProblemInlineNotesLevel problemInlineNotesLevel() const = 0;
    virtual bool boldDeclarations() const = 0;

    virtual bool showMultiLineSelectionInformation() const = 0;

    virtual QStringList todoMarkerWords() const = 0;

Q_SIGNALS:
    void settingsChanged(ICompletionSettings*);
};

}

#endif
