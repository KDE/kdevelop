/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H

#include <QProcess>
#include <outputview/outputjob.h>

#include "externalscriptitem.h"

#include <KTextEditor/Range>
#include <QUrl>

namespace KDevelop {
class ProcessLineMaker;
class OutputModel;
class Document;
}

namespace KTextEditor {
class Document;
}

class KProcess;
class ExternalScriptPlugin;

class ExternalScriptJob
    : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    ExternalScriptJob(ExternalScriptItem* item, const QUrl& url, ExternalScriptPlugin* parent);
    void start() override;
    KDevelop::OutputModel* model();

protected:
    bool doKill() override;

private Q_SLOTS:
    void processError(QProcess::ProcessError);
    void processFinished(int, QProcess::ExitStatus);

    void receivedStdoutLines(const QStringList& lines);
    void receivedStderrLines(const QStringList& lines);

private:
    void appendLine(const QString& l);

    KProcess* m_proc;
    KDevelop::ProcessLineMaker* m_lineMaker;
    ExternalScriptItem::OutputMode m_outputMode;
    ExternalScriptItem::InputMode m_inputMode;
    ExternalScriptItem::ErrorMode m_errorMode;
    int m_filterMode;
    KTextEditor::Document* m_document;
    QUrl m_url;
    /// invalid when whole doc should be replaced
    KTextEditor::Range m_selectionRange;
    KTextEditor::Cursor m_cursorPosition;
    bool m_showOutput;

    QStringList m_stdout;
    QStringList m_stderr;
};

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H
