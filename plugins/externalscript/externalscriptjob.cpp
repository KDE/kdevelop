/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "externalscriptjob.h"

#include "externalscriptitem.h"
#include "externalscriptplugin.h"
#include <debug.h>

#include <QFileInfo>
#include <QApplication>

#include <KProcess>
#include <KLocalizedString>
#include <KShell>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/processlinemaker.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <serialization/indexedstring.h>
#include <sublime/message.h>
#include <util/path.h>

using namespace KDevelop;

ExternalScriptJob::ExternalScriptJob(ExternalScriptItem* item, const QUrl& url, ExternalScriptPlugin* parent)
    : KDevelop::OutputJob(parent)
    , m_proc(nullptr)
    , m_lineMaker(nullptr)
    , m_outputMode(item->outputMode())
    , m_inputMode(item->inputMode())
    , m_errorMode(item->errorMode())
    , m_filterMode(item->filterMode())
    , m_document(nullptr)
    , m_url(url)
    , m_selectionRange(KTextEditor::Range::invalid())
    , m_showOutput(item->showOutput())
{
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "creating external script job";

    setCapabilities(Killable);
    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    auto* model = new KDevelop::OutputModel;
    model->setFilteringStrategy(static_cast<KDevelop::OutputModel::OutputFilterStrategy>(m_filterMode));
    setModel(model);

    setDelegate(new KDevelop::OutputDelegate);

    // also merge when error mode "equals" output mode
    if ((m_outputMode == ExternalScriptItem::OutputInsertAtCursor
         && m_errorMode == ExternalScriptItem::ErrorInsertAtCursor) ||
        (m_outputMode == ExternalScriptItem::OutputReplaceDocument
         && m_errorMode == ExternalScriptItem::ErrorReplaceDocument) ||
        (m_outputMode == ExternalScriptItem::OutputReplaceSelectionOrDocument
         && m_errorMode == ExternalScriptItem::ErrorReplaceSelectionOrDocument) ||
        (m_outputMode == ExternalScriptItem::OutputReplaceSelectionOrInsertAtCursor
         && m_errorMode == ExternalScriptItem::ErrorReplaceSelectionOrInsertAtCursor) ||
        // also these two otherwise they clash...
        (m_outputMode == ExternalScriptItem::OutputReplaceSelectionOrInsertAtCursor
         && m_errorMode == ExternalScriptItem::ErrorReplaceSelectionOrDocument) ||
        (m_outputMode == ExternalScriptItem::OutputReplaceSelectionOrDocument
         && m_errorMode == ExternalScriptItem::ErrorReplaceSelectionOrInsertAtCursor)) {
        m_errorMode = ExternalScriptItem::ErrorMergeOutput;
    }

    KTextEditor::View* view = KDevelop::ICore::self()->documentController()->activeTextDocumentView();

    if (m_outputMode != ExternalScriptItem::OutputNone || m_inputMode != ExternalScriptItem::InputNone
        || m_errorMode != ExternalScriptItem::ErrorNone) {
        if (!view) {
            const QString messageText =
                i18n("Cannot run script '%1' since it tries to access "
                     "the editor contents but no document is open.", item->text());
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
            return;
        }

        m_document = view->document();

        connect(m_document, &KTextEditor::Document::aboutToClose, this, [&] {
            kill();
        });

        m_selectionRange = view->selectionRange();
        m_cursorPosition = view->cursorPosition();
    }

    if (item->saveMode() == ExternalScriptItem::SaveCurrentDocument && view) {
        if (!view->document()->documentSave()) {
            return; // canceled by the user
        }
    } else if (item->saveMode() == ExternalScriptItem::SaveAllDocuments) {
        const auto documents = KDevelop::ICore::self()->documentController()->openDocuments();
        for (KDevelop::IDocument* doc : documents) {
            if (!doc->save()) {
                return; // canceled by the user
            }
        }
    }

    QString command = item->command();
    QString workingDir = item->workingDirectory();

    if (item->performParameterReplacement())
        command.replace(QLatin1String("%i"), QString::number(QCoreApplication::applicationPid()));

    if (!m_url.isEmpty()) {
        const QUrl url = m_url;

        KDevelop::ProjectFolderItem* folder = nullptr;
        if (KDevelop::ICore::self()->projectController()->findProjectForUrl(url)) {
            QList<KDevelop::ProjectFolderItem*> folders =
                KDevelop::ICore::self()->projectController()->findProjectForUrl(url)->foldersForPath(KDevelop::IndexedString(
                                                                                                         url));
            if (!folders.isEmpty()) {
                folder = folders.first();
            }
        }

        if (folder) {
            if (folder->path().isLocalFile() && workingDir.isEmpty()) {
                ///TODO: make configurable, use fallback to project dir
                workingDir = folder->path().toLocalFile();
            }

            ///TODO: make those placeholders escapeable
            if (item->performParameterReplacement()) {
                command.replace(QLatin1String("%d"), KShell::quoteArg(m_url.toString(QUrl::PreferLocalFile)));

                if (KDevelop::IProject* project =
                        KDevelop::ICore::self()->projectController()->findProjectForUrl(m_url)) {
                    command.replace(QLatin1String("%p"), project->path().pathOrUrl());
                }
            }
        } else {
            if (m_url.isLocalFile() && workingDir.isEmpty()) {
                ///TODO: make configurable, use fallback to project dir
                workingDir = view->document()->url().adjusted(QUrl::RemoveFilename).toLocalFile();
            }

            ///TODO: make those placeholders escapeable
            if (item->performParameterReplacement()) {
                command.replace(QLatin1String("%u"), KShell::quoteArg(m_url.toString()));

                ///TODO: does that work with remote files?
                QFileInfo info(m_url.toString(QUrl::PreferLocalFile));

                command.replace(QLatin1String("%f"), KShell::quoteArg(info.filePath()));
                command.replace(QLatin1String("%b"), KShell::quoteArg(info.baseName()));
                command.replace(QLatin1String("%n"), KShell::quoteArg(info.fileName()));
                command.replace(QLatin1String("%d"), KShell::quoteArg(info.path()));

                if (view->document()) {
                    command.replace(QLatin1String("%c"),
                                    KShell::quoteArg(QString::number(view->cursorPosition().column())));
                    command.replace(QLatin1String("%l"), KShell::quoteArg(QString::number(
                                                                              view->cursorPosition().line())));
                }

                if (view->document() && view->selection()) {
                    command.replace(QLatin1String("%s"), KShell::quoteArg(view->selectionText()));
                }

                if (KDevelop::IProject* project =
                        KDevelop::ICore::self()->projectController()->findProjectForUrl(m_url)) {
                    command.replace(QLatin1String("%p"), project->path().pathOrUrl());
                }
            }
        }
    }

    m_proc = new KProcess(this);
    if (!workingDir.isEmpty()) {
        m_proc->setWorkingDirectory(workingDir);
    }
    m_lineMaker = new ProcessLineMaker(m_proc, this);
    connect(m_lineMaker, &ProcessLineMaker::receivedStdoutLines,
            model, &OutputModel::appendLines);
    connect(m_lineMaker, &ProcessLineMaker::receivedStdoutLines,
            this, &ExternalScriptJob::receivedStdoutLines);
    connect(m_lineMaker, &ProcessLineMaker::receivedStderrLines,
            model, &OutputModel::appendLines);
    connect(m_lineMaker, &ProcessLineMaker::receivedStderrLines,
            this, &ExternalScriptJob::receivedStderrLines);
    connect(m_proc, &QProcess::errorOccurred,
            this, &ExternalScriptJob::processError);
    connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ExternalScriptJob::processFinished);

    // Now setup the process parameters
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "setting command:" << command;

    if (m_errorMode == ExternalScriptItem::ErrorMergeOutput) {
        m_proc->setOutputChannelMode(KProcess::MergedChannels);
    } else {
        m_proc->setOutputChannelMode(KProcess::SeparateChannels);
    }
    m_proc->setShellCommand(command);

    setObjectName(command);
}

void ExternalScriptJob::start()
{
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "launching?" << m_proc;

    if (m_proc) {
        if (m_showOutput) {
            startOutput();
        }
        appendLine(i18n("Running external script: %1", m_proc->program().join(QLatin1Char(' '))));
        m_proc->start();

        if (m_inputMode != ExternalScriptItem::InputNone) {
            QString inputText;

            switch (m_inputMode) {
            case ExternalScriptItem::InputNone:
                // do nothing;
                break;
            case ExternalScriptItem::InputSelectionOrNone:
                if (m_selectionRange.isValid()) {
                    inputText = m_document->text(m_selectionRange);
                } // else nothing
                break;
            case ExternalScriptItem::InputSelectionOrDocument:
                if (m_selectionRange.isValid()) {
                    inputText = m_document->text(m_selectionRange);
                } else {
                    inputText = m_document->text();
                }
                break;
            case ExternalScriptItem::InputDocument:
                inputText = m_document->text();
                break;
            }

            ///TODO: what to do with the encoding here?
            ///      maybe ask Christoph for what kate returns...
            m_proc->write(inputText.toUtf8());

            m_proc->closeWriteChannel();
        }
    } else {
        qCDebug(PLUGIN_EXTERNALSCRIPT) << "No process, something went wrong when creating the job";
        // No process means we've returned early on from the constructor, some bad error happened
        emitResult();
    }
}

bool ExternalScriptJob::doKill()
{
    if (m_proc) {
        m_proc->kill();
        appendLine(i18n("*** Killed Application ***"));
    }

    return true;
}

void ExternalScriptJob::processFinished(int exitCode, QProcess::ExitStatus status)
{
    m_lineMaker->flushBuffers();

    if (exitCode == 0 && status == QProcess::NormalExit) {
        if (m_outputMode != ExternalScriptItem::OutputNone) {
            if (!m_stdout.isEmpty()) {
                QString output = m_stdout.join(QLatin1Char('\n'));
                switch (m_outputMode) {
                case ExternalScriptItem::OutputNone:
                    // do nothing;
                    break;
                case ExternalScriptItem::OutputCreateNewFile:
                    KDevelop::ICore::self()->documentController()->openDocumentFromText(output);
                    break;
                case ExternalScriptItem::OutputInsertAtCursor:
                    m_document->insertText(m_cursorPosition, output);
                    break;
                case ExternalScriptItem::OutputReplaceSelectionOrInsertAtCursor:
                    if (m_selectionRange.isValid()) {
                        m_document->replaceText(m_selectionRange, output);
                    } else {
                        m_document->insertText(m_cursorPosition, output);
                    }
                    break;
                case ExternalScriptItem::OutputReplaceSelectionOrDocument:
                    if (m_selectionRange.isValid()) {
                        m_document->replaceText(m_selectionRange, output);
                    } else {
                        m_document->setText(output);
                    }
                    break;
                case ExternalScriptItem::OutputReplaceDocument:
                    m_document->setText(output);
                    break;
                }
            }
        }
        if (m_errorMode != ExternalScriptItem::ErrorNone && m_errorMode != ExternalScriptItem::ErrorMergeOutput) {
            QString output = m_stderr.join(QLatin1Char('\n'));

            if (!output.isEmpty()) {
                switch (m_errorMode) {
                case ExternalScriptItem::ErrorNone:
                case ExternalScriptItem::ErrorMergeOutput:
                    // do nothing;
                    break;
                case ExternalScriptItem::ErrorCreateNewFile:
                    KDevelop::ICore::self()->documentController()->openDocumentFromText(output);
                    break;
                case ExternalScriptItem::ErrorInsertAtCursor:
                    m_document->insertText(m_cursorPosition, output);
                    break;
                case ExternalScriptItem::ErrorReplaceSelectionOrInsertAtCursor:
                    if (m_selectionRange.isValid()) {
                        m_document->replaceText(m_selectionRange, output);
                    } else {
                        m_document->insertText(m_cursorPosition, output);
                    }
                    break;
                case ExternalScriptItem::ErrorReplaceSelectionOrDocument:
                    if (m_selectionRange.isValid()) {
                        m_document->replaceText(m_selectionRange, output);
                    } else {
                        m_document->setText(output);
                    }
                    break;
                case ExternalScriptItem::ErrorReplaceDocument:
                    m_document->setText(output);
                    break;
                }
            }
        }

        appendLine(i18n("*** Exited normally ***"));
    } else {
        if (status == QProcess::NormalExit)
            appendLine(i18n("*** Exited with return code: %1 ***", QString::number(exitCode)));
        else
        if (error() == KJob::KilledJobError)
            appendLine(i18n("*** Process aborted ***"));
        else
            appendLine(i18n("*** Crashed with return code: %1 ***", QString::number(exitCode)));
    }

    qCDebug(PLUGIN_EXTERNALSCRIPT) << "Process done";

    emitResult();
}

void ExternalScriptJob::processError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        setError(-1);
        QString errmsg =  i18n("*** Could not start program '%1'. Make sure that the "
                               "path is specified correctly ***", m_proc->program().join(QLatin1Char(' ')));
        appendLine(errmsg);
        setErrorText(errmsg);
        emitResult();
    }

    qCDebug(PLUGIN_EXTERNALSCRIPT) << "Process error";
}

void ExternalScriptJob::appendLine(const QString& l)
{
    if (KDevelop::OutputModel* m = model()) {
        m->appendLine(l);
    }
}

KDevelop::OutputModel* ExternalScriptJob::model()
{
    return qobject_cast<KDevelop::OutputModel*>(OutputJob::model());
}

void ExternalScriptJob::receivedStderrLines(const QStringList& lines)
{
    m_stderr += lines;
}

void ExternalScriptJob::receivedStdoutLines(const QStringList& lines)
{
    m_stdout += lines;
}

#include "moc_externalscriptjob.cpp"
