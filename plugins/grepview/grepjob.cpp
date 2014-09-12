/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2008 by Hamish Rodda                                        *
*   rodda@kde.org                                                         *
*   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepjob.h"
#include "grepoutputmodel.h"
#include "greputil.h"

#include <QList>
#include <QRegExp>
#include <QTextDocument>
#include <QTextStream>

#include <KLocalizedString>
#include <kencodingprober.h>

#include <serialization/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <language/codegen/documentchangeset.h>

using namespace KDevelop;


GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re)
{
    GrepOutputItem::List res;
    QFile file(filename);
    
    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 0;
    
    
    // detect encoding (unicode files can be feed forever, stops when confidence reachs 99%
    KEncodingProber prober;
    while(!file.atEnd() && prober.state() == KEncodingProber::Probing && prober.confidence() < 0.99) {
        prober.feed(file.read(0xFF));
    }
        
    // reads file with detected encoding
    file.seek(0);
    QTextStream stream(&file);
    if(prober.confidence()>0.7)
        stream.setCodec(prober.encoding());
    while( !stream.atEnd() )
    {
        QString data = stream.readLine();
        
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
                KTextEditor::Range(lineno, start, lineno, end),
                re.cap(0), QString()));
            
            res << GrepOutputItem(change, data, false);
            offset = end;
        }
        lineno++;
    }
    file.close();
    return res;
}

GrepJob::GrepJob( QObject* parent )
    : KJob( parent )
    , m_workState(WorkIdle)
    , m_fileIndex(0)
    , m_useProjectFilesFlag(false)
    , m_regexpFlag(true)
    , m_caseSensitiveFlag(true)
    , m_depthValue(-1)
    , m_findSomething(false)
{
    setCapabilities(Killable);
    KDevelop::ICore::self()->uiController()->registerStatus(this);
    
    connect(this, SIGNAL(result(KJob*)), this, SLOT(testFinishState(KJob*)));
}

QString GrepJob::statusName() const
{
    return i18n("Find in Files");
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
        m_errorMessage = i18n("Search aborted");
        emitResult();
        return;
    }
    if(m_fileList.isEmpty())
    {
        m_workState = WorkIdle;
        emit hideProgress(this);
        emit clearMessage(this);
        m_errorMessage = i18n("No files found matching the wildcard patterns");
        //model()->slotFailed();
        emitResult();
        return;
    }
    
    if(!m_regexpFlag) 
    {
        m_patternString = QRegExp::escape(m_patternString);
    }

    if(m_regexpFlag && QRegExp(m_patternString).captureCount() > 0)
    {
        m_workState = WorkIdle;
        emit hideProgress(this);
        emit clearMessage(this);
        m_errorMessage = i18nc("Capture is the text which is \"captured\" with () in regular expressions "
                                    "see http://doc.trolltech.com/qregexp.html#capturedTexts",
                                    "Captures are not allowed in pattern string");
        emitResult();
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
    
    m_outputModel->setRegExp(m_regExp);
    m_outputModel->setReplacementTemplate(m_replacementTemplateString);


    emit showMessage(this, i18np("Searching for <b>%2</b> in one file",
                                 "Searching for <b>%2</b> in %1 files",
                                 m_fileList.length(),
                                 m_regExp.pattern().toHtmlEscaped()));

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
            m_findThread = new GrepFindFilesThread(this, m_directoryChoice, m_depthValue, m_filesString, m_excludeString, m_useProjectFilesFlag);
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
                    GrepOutputItem::List items = grepFile(file, m_regExp);

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
        case WorkCancelled:            
            emit hideProgress(this);
            emit clearMessage(this);
            emit showErrorMessage(i18n("Search aborted"), 5000);
            emitResult();
            break;
    }
}

void GrepJob::start()
{
    if(m_workState!=WorkIdle)
        return;
    
    m_fileList.clear();
    m_workState = WorkIdle;
    m_fileIndex = 0;

    m_findSomething = false;
    m_outputModel->clear();

    qRegisterMetaType<GrepOutputItem::List>();
    connect(this, SIGNAL(foundMatches(QString,GrepOutputItem::List)),
            m_outputModel, SLOT(appendOutputs(QString,GrepOutputItem::List)), Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
}

bool GrepJob::doKill()
{
    if(m_workState!=WorkIdle && !m_findThread.isNull())
    {
        m_workState = WorkIdle;
        m_findThread->tryAbort();
        return false;
    }
    else
    {
        m_workState = WorkCancelled;
    }
    return true;
}

void GrepJob::testFinishState(KJob *job)
{
    if(!job->error())
    {
        if (!m_errorMessage.isEmpty()) {
            emit showErrorMessage(i18n("Failed: %1", m_errorMessage));
        }
        else if (!m_findSomething) {
            emit showMessage(this, i18n("No results found"));
        }
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

void GrepJob::setDirectoryChoice(const QList<KUrl>& choice)
{
    m_directoryChoice = choice;
}

void GrepJob::setCaseSensitive(bool caseSensitive)
{
    m_caseSensitiveFlag = caseSensitive;
}

void GrepJob::setDepth(int depth)
{
    m_depthValue = depth;
}

void GrepJob::setRegexpFlag(bool regexpFlag)
{
    m_regexpFlag = regexpFlag;
}

void GrepJob::setProjectFilesFlag(bool projectFilesFlag)
{
    m_useProjectFilesFlag = projectFilesFlag;
}

void GrepJob::setPatternString(const QString& patternString)
{
    m_patternString = patternString;

    setObjectName(i18n("Grep: %1", m_patternString));
}

