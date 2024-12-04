/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grepjob.h"
#include "grepfindthread.h"
#include "grepoutputmodel.h"
#include "greputil.h"

#include "debug.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <KEncodingProber>
#include <KLocalizedString>

#include <serialization/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

using namespace KDevelop;

QDebug operator<<(QDebug debug, const GrepJobSettings& s)
{
    const QDebugStateSaver saver(debug);

    debug.nospace() << '{';

    bool firstDataMember = true;
    const auto printDataMember = [&debug, &firstDataMember](const char* name, const auto& value) {
        if (firstDataMember) {
            firstDataMember = false;
        } else {
            // Separate the members with vertical bars,
            // because commas and semicolons separate items within string data members.
            debug << " | ";
        }
        debug << name << ": " << value;
    };

#define p(memberName) printDataMember(#memberName, s.memberName)
    p(projectFilesOnly);
    p(caseSensitive);
    p(regexp);

    p(depth);

    p(pattern);
    p(searchTemplate);
    p(replacementTemplate);
    p(files);
    p(exclude);
    p(searchPaths);
#undef p

    debug << '}';

    return debug;
}

GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re)
{
    GrepOutputItem::List res;
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 0;

    // detect encoding (unicode files can be fed forever, so stop when confidence reaches 99%)
    KEncodingProber prober;
    while(!file.atEnd() && prober.state() == KEncodingProber::Probing && prober.confidence() < 0.99) {
        prober.feed(file.read(0xFF));
    }

    // reads file with detected encoding
    file.seek(0);
    // TODO: consider using QIODevice::readLine() and QStringDecoder in place of QTextStream.
    // The reason is the following paragraph in the documentation of QStringConverter::encodingForName():
    //      If the name is not the name of a codec listed in the Encoding enumeration, std::nullopt
    //      is returned. Such a name may, none the less, be accepted by the QStringConverter
    //      constructor when Qt is built with ICU, if ICU provides a converter with the given name.
    // The code inside the following `if` block replaced the line `stream.setCodec(prober.encoding().constData());`
    // during porting from Qt 5 to Qt 6. The function QTextStream::setCodec(const char *codecName)
    // removed in Qt 6 had probably supported ICU codec names.
    QTextStream stream(&file);
    if (prober.confidence() > 0.7) {
        const auto encoding = QStringConverter::encodingForName(prober.encoding().constData());
        if (encoding) {
            stream.setEncoding(*encoding);
        }
    }
    while( !stream.atEnd() )
    {
        QString data = stream.readLine();

        // remove line terminators (in order to not match them)
        for (int pos = data.length()-1; pos >= 0 && (data[pos] == QLatin1Char('\r') || data[pos] == QLatin1Char('\n')); pos--) {
            data.chop(1);
        }

        int offset = 0;
        // allow empty string matching result in an infinite loop !
        while( re.indexIn(data, offset)!=-1 && re.cap(0).length() > 0 )
        {
            int start = re.pos(0);
            int end = start + re.cap(0).length();

            DocumentChangePointer change = DocumentChangePointer(new DocumentChange(
                IndexedString(filename),
                KTextEditor::Range(lineno, start, lineno, end),
                re.cap(0), QString()));

            res->push_back(new GrepOutputItem(change, data, false));
            offset = end;
        }
        lineno++;
    }
    file.close();
    return res;
}

GrepJob::GrepJob( QObject* parent )
    : KJob( parent )
    , m_workState(WorkUnstarted)
    , m_fileIndex(0)
    , m_findThread(nullptr)
    , m_findSomething(false)
{
    setCapabilities(Killable);
    KDevelop::ICore::self()->uiController()->registerStatus(this);

    connect(this, &GrepJob::result, this, &GrepJob::testFinishState);
}

GrepJob::~GrepJob()
{
    Q_ASSERT(m_workState == WorkDead);
}

QString GrepJob::statusName() const
{
    return i18n("Find in Files");
}

void GrepJob::slotFindFinished()
{
    Q_ASSERT(m_findThread && m_findThread->isFinished());

    if (m_workState == WorkCancelled) {
        dieAfterCancellation();
        return;
    }
    Q_ASSERT(m_workState == WorkCollectFiles);

    m_fileList = m_findThread->takeFiles();
    m_findThread->deleteLater();
    m_findThread = nullptr;

    if(m_fileList.isEmpty())
    {
        m_errorMessage = i18n("No files found matching the wildcard patterns");
        die();
        return;
    }

    if(!m_settings.regexp)
    {
        m_settings.pattern = QRegExp::escape(m_settings.pattern);
    }

    if(m_settings.regexp && QRegExp(m_settings.pattern).captureCount() > 0)
    {
        m_errorMessage = i18nc("Capture is the text which is \"captured\" with () in regular expressions "
                                    "see https://doc.qt.io/qt-5/qregexp.html#capturedTexts",
                                    "Captures are not allowed in pattern string");
        die();
        return;
    }

    QString pattern = substitudePattern(m_settings.searchTemplate, m_settings.pattern);
    m_regExp.setPattern(pattern);
    m_regExp.setPatternSyntax(QRegExp::RegExp2);
    m_regExp.setCaseSensitivity( m_settings.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
    if(pattern == QRegExp::escape(pattern))
    {
        // enable wildcard mode when possible
        // if pattern has already been escaped (raw text search) a second escape will result in a different string anyway
        m_regExp.setPatternSyntax(QRegExp::Wildcard);
    }

    if (m_outputModel) {
        m_outputModel->setRegExp(m_regExp);
        m_outputModel->setReplacementTemplate(m_settings.replacementTemplate);
    }

    emit showMessage(this, i18np("Searching for <b>%2</b> in one file",
                                 "Searching for <b>%2</b> in %1 files",
                                 m_fileList.length(),
                                 m_regExp.pattern().toHtmlEscaped()));

    m_workState = WorkGrep;
    QMetaObject::invokeMethod( this, "slotWork", Qt::QueuedConnection);
}

void GrepJob::slotWork()
{
    Q_ASSERT(!m_findThread);

    switch(m_workState)
    {
        case WorkUnstarted:
        case WorkDead:
            Q_UNREACHABLE();
            break;
        case WorkStarting:
            m_workState = WorkCollectFiles;
            emit showProgress(this, 0,0,0);
            QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
            break;
        case WorkCollectFiles:
            m_findThread = new GrepFindFilesThread(this, m_directoryChoice, m_settings.depth, m_settings.files, m_settings.exclude, m_settings.projectFilesOnly);
            emit showMessage(this, i18n("Collecting files..."));
            connect(m_findThread, &GrepFindFilesThread::finished, this, &GrepJob::slotFindFinished);
            m_findThread->start();
            break;
        case WorkGrep:
            if(m_fileIndex < m_fileList.length())
            {
                emit showProgress(this, 0, m_fileList.length(), m_fileIndex);
                if(m_fileIndex < m_fileList.length()) {
                    QString file = m_fileList[m_fileIndex].toLocalFile();
                    GrepOutputItem::List items = grepFile(file, m_regExp);

                    if (!items->empty()) {
                        m_findSomething = true;
                        if (m_outputModel) {
                            m_outputModel->appendOutputs(file, std::move(items));
                        }
                    }

                    m_fileIndex++;
                }
                QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
            }
            else
            {
                die();
            }
            break;
        case WorkCancelled:
            dieAfterCancellation();
            break;
    }
}

void GrepJob::die()
{
    emit hideProgress(this);
    emit clearMessage(this);
    m_workState = WorkDead;
    emitResult();
}

void GrepJob::dieAfterCancellation()
{
    Q_ASSERT(m_workState == WorkCancelled);
    m_errorMessage = i18n("Search aborted");
    die();
}

void GrepJob::start()
{
    if (m_workState != WorkUnstarted) {
        qCWarning(PLUGIN_GREPVIEW) << "cannot start a grep job more than once, current state:" << m_workState;
        return;
    }

    m_workState = WorkStarting;

    QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
}

bool GrepJob::doKill()
{
    if (m_workState == WorkUnstarted || m_workState == WorkDead) {
        m_workState = WorkDead;
        return true;
    }
    if (m_workState != WorkCancelled) {
        if (m_findThread) {
            Q_ASSERT(m_workState == WorkCollectFiles);
            m_findThread->tryAbort();
        }
        m_workState = WorkCancelled;
    }
    // Do not let KJob finish immediately if the state was neither Unstarted nor Dead:
    // * If m_findThread != nullptr, let it finish first.
    // * Otherwise, slotWork() is about to be invoked. Don't want this to be destroyed by KJob before that happens.
    return false;
}

void GrepJob::testFinishState(KJob *job)
{
    Q_ASSERT(m_workState == WorkDead);
    if(!job->error())
    {
        if (!m_errorMessage.isEmpty()) {
            emit showErrorMessage(i18n("Failed: %1", m_errorMessage));
        }
        else if (!m_findSomething) {
            emit showMessage(this, i18n("No results found"), 3000);
        }
    }
}

void GrepJob::setOutputModel(GrepOutputModel* model)
{
    Q_ASSERT(model);
    m_outputModel = model;
    model->setJob(*this);
}

void GrepJob::setDirectoryChoice(const QList<QUrl>& choice)
{
    m_directoryChoice = choice;
}

void GrepJob::setSettings(const GrepJobSettings& settings)
{
    m_settings = settings;

    setObjectName(i18n("Grep: %1", m_settings.pattern));
}

#include "moc_grepjob.cpp"
