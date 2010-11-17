/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2008 by Hamish Rodda                                        *
*   rodda@kde.org                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepjob.h"
#include "grepoutputmodel.h"
#include "grepoutputview.h"

#include <QList>
#include <QRegExp>
#include <QKeySequence>
#include <QTextDocument>

#include <kdebug.h>
#include <klocale.h>

#include <language/duchain/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <language/codegen/documentchangeset.h>

using namespace KDevelop;


GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re, const QString &repl, bool replace)
{
    GrepOutputItem::List res;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 0;
    while( !file.atEnd() )
    {
        QByteArray data = file.readLine();
        
        // remove line terminators (in order to not match them)
        for(int pos = data.length()-1; pos >= 0 && (data[pos] == '\r' || data[pos] == '\n'); pos--)
        {
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
                SimpleRange(lineno, start, lineno, end),
                re.cap(0), re.cap(0).replace(re, repl)));
            
            res << GrepOutputItem(change, QString(data), replace);
            offset = end;
        }
        lineno++;
    }
    file.close();
    return res;
}

GrepJob::GrepJob( QObject* parent )
    : KJob( parent ), m_workState(WorkIdle)
{
    setCapabilities(Killable);
    KDevelop::ICore::self()->uiController()->registerStatus(this);
    
    connect(this, SIGNAL(result(KJob *)), this, SLOT(testFinishState(KJob *)));

    //FIXME only for benchmarks
    connect(this, SIGNAL(finished(KJob *)), this, SLOT(doBench()));
}

QString GrepJob::statusName() const
{
    return i18n("Find in Files");
}

QString substitudePattern(const QString& pattern, const QString& searchString)
{
    QString subst = searchString;
    QString result;
    bool expectEscape = false;
    foreach(const QChar &ch, pattern)
    {
        if(expectEscape)
        {
            expectEscape = false;
            if(ch == '%')
                result.append('%');
            else if(ch == 's')
                result.append(subst);
            else
                result.append('%').append(ch);
        }
        else if(ch == '%')
            expectEscape = true;
        else
            result.append(ch);
    }
    // kDebug() << "Pattern substituded:" << pattern << "+" << searchString << "=" << result;
    return result;
}

void GrepJob::slotFindFinished()
{
    if(m_findThread && !m_findThread->triesToAbort())
    {
        m_fileList = m_findThread->files();
        delete m_findThread;
    }
    else
    {
        m_fileList.clear();
        emit hideProgress(this);
        emit clearMessage(this);
        emit showErrorMessage(i18n("Find in Files aborted"), 5000);
        emitResult();
        return;
    }
    if(m_fileList.isEmpty())
    {
        m_workState = WorkIdle;
        emit hideProgress(this);
        emit clearMessage(this);
        emit showErrorMessage(i18n("No files found matching the wildcard patterns"), 5000);
        //model()->slotFailed();
        emitResult();
        return;
    }
    
    if(!m_regexpFlag) 
    {
        m_patternString = QRegExp::escape(m_patternString);
    }
    
    if(m_replaceFlag && m_regexpFlag && QRegExp(m_patternString).captureCount() > 0)
    {
        emit showErrorMessage(i18n("Captures are not allowed in pattern string"), 5000);
        return;
    }
    
    QString pattern = substitudePattern(m_templateString, m_patternString);
    m_regExp.setPattern(pattern);
    m_regExp.setPatternSyntax(QRegExp::RegExp2);
    m_regExp.setCaseSensitivity( m_caseSensitiveFlag ? Qt::CaseSensitive : Qt::CaseInsensitive );
    if(pattern == QRegExp::escape(pattern))
    {
        // enable wildcard mode when possible
        // if pattern has already been escaped (raw text serch) a second escape will result in a different string anyway
        m_regExp.setPatternSyntax(QRegExp::Wildcard);
    }
    
    // backslashes can be sprecial chars
    QString replacement = (m_regExp.patternSyntax() == QRegExp::Wildcard) ? m_replaceString : m_replaceString.replace("\\", "\\\\");
    m_finalReplacement = substitudePattern(m_replacementTemplateString, replacement);
    
    m_outputModel->setRegExp(m_regExp);

    if(m_replaceFlag)
    {
        emit showMessage(this, i18np("Replace <b>%2</b> by <b>%3</b> in one file",
                                     "Replace <b>%2</b> by <b>%3</b> in %1 files",
                                     m_fileList.length(), 
                                     Qt::escape(m_regExp.pattern()), 
                                     Qt::escape(m_finalReplacement)));
    }
    else
    {
        emit showMessage(this, i18np("Searching for <b>%2</b> in one file",
                                     "Searching for <b>%2</b> in %1 files",
                                     m_fileList.length(), 
                                     Qt::escape(m_regExp.pattern())));
    }

    m_workState = WorkGrep;
    QMetaObject::invokeMethod( this, "slotWork", Qt::QueuedConnection);
}

void GrepJob::slotWork()
{
    switch(m_workState)
    {
        case WorkIdle:
            m_workState = WorkCollectFiles;
            m_fileIndex = 0;
            emit showProgress(this, 0,0,0);
            QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
            break;
        case WorkCollectFiles:
            m_findThread = new GrepFindFilesThread(this, m_directory, m_recursiveFlag, m_filesString, m_excludeString, m_useProjectFilesFlag);
            emit showMessage(this, i18n("Collecting files..."));
            connect(m_findThread, SIGNAL(finished()), this, SLOT(slotFindFinished()));
            m_findThread->start();
            break;
        case WorkGrep:
            if(m_fileIndex < m_fileList.length())
            {
                emit showProgress(this, 0, m_fileList.length(), m_fileIndex);
                if(m_fileIndex < m_fileList.length()) {
                    QString file = m_fileList[m_fileIndex].toLocalFile();
                    GrepOutputItem::List items = grepFile(file, m_regExp, m_finalReplacement, m_replaceFlag);

                    if(!items.isEmpty())
                    {
                        m_findSomething = true;
                        emit foundMatches(file, items);
                    }

                    m_fileIndex++;
                }
                QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
            }
            else
            {
                emit hideProgress(this);
                emit clearMessage(this);
                m_workState = WorkIdle;
                //model()->slotCompleted();
                emitResult();
            }
            break;
    }
}

void GrepJob::start()
{
    if(m_workState!=WorkIdle)
        return;
    
    //FIXME: only for benchmarks
    m_timer.start();
    
    m_fileList.clear();
    m_workState = WorkIdle;
    m_fileIndex = 0;

    m_findSomething = false;
    m_outputModel->clear();

    qRegisterMetaType<GrepOutputItem::List>();
    connect(this, SIGNAL(foundMatches(QString, GrepOutputItem::List)),
            m_outputModel, SLOT(appendOutputs(QString, GrepOutputItem::List)), Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
}

//FIXME: only for benchmarks
void GrepJob::doBench()
{
    qDebug() << "Grep done in " << m_timer.elapsed() << " ms";
}

bool GrepJob::doKill()
{
    if(m_workState!=WorkIdle && !m_findThread.isNull())
    {
        m_workState = WorkIdle;
        m_findThread->tryAbort();
        return false;
    }
    return true;
}

void GrepJob::testFinishState(KJob *job)
{
    if(!job->error())
    {
        if(!m_findSomething) emit showMessage(this, i18n("No results found"));
    }
}

void GrepJob::setOutputModel(GrepOutputModel* model)
{
    m_outputModel = model;
}

void GrepJob::setTemplateString(const QString& templateString)
{
    m_templateString = templateString;
}

void GrepJob::setReplacementTemplateString(const QString &replTmplString)
{
    m_replacementTemplateString = replTmplString;
}

void GrepJob::setFilesString(const QString& filesString)
{
    m_filesString = filesString;
}

void GrepJob::setExcludeString(const QString& excludeString)
{
    m_excludeString = excludeString;
}

void GrepJob::setDirectory(const KUrl& directory)
{
    m_directory = directory;
}

void GrepJob::setCaseSensitive(bool caseSensitive)
{
    m_caseSensitiveFlag = caseSensitive;
}

void GrepJob::setRecursive(bool recursive)
{
    m_recursiveFlag = recursive;
}

void GrepJob::setRegexpFlag(bool regexpFlag)
{
    m_regexpFlag = regexpFlag;
}

void GrepJob::setProjectFilesFlag(bool projectFilesFlag)
{
    m_useProjectFilesFlag = projectFilesFlag;
}

void GrepJob::setReplaceFlag(bool replaceFlag)
{
    m_replaceFlag = replaceFlag;
}

void GrepJob::setReplaceString(const QString& replaceString)
{
    m_replaceString = replaceString;
}

void GrepJob::setPatternString(const QString& patternString)
{
    m_patternString = patternString;

    setObjectName(i18n("Grep: %1", m_patternString));
}

#include "grepjob.moc"
