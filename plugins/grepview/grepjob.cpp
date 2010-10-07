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

#include <QWhatsThis>
#include <QList>
#include <QRegExp>
#include <QKeySequence>

#include <kprocess.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <project/projectmodel.h>
#include <util/processlinemaker.h>

#include <language/duchain/indexedstring.h>
#include <interfaces/iuicontroller.h>

#include "grepoutputdelegate.h"

static GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re)
{
    GrepOutputItem::List res;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 1;
    QByteArray data = file.readLine();
    while( !data.isNull() && file.error()==QFile::NoError )
    {
        if( re.indexIn(data)!=-1 )
            res << GrepOutputItem(filename, lineno, QString(data).trimmed());
        lineno++;
        data = file.readLine();
    }
    file.close();
    return res;
}

static GrepOutputItem::List grepFileSimple(const QString &filename, const QString &pattern, bool caseSense)
{
    GrepOutputItem::List res;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 1;
    QByteArray data = file.readLine();
    while( !data.isNull() && file.error()==QFile::NoError )
    {
        QString dataStr = data;
        if( dataStr.contains(pattern, caseSense ? Qt::CaseSensitive : Qt::CaseInsensitive) )
            res << GrepOutputItem(filename, lineno, dataStr.trimmed());
        lineno++;
        data = file.readLine();
    }
    file.close();
    return res;
}

GrepJob::GrepJob( QObject* parent )
    : KDevelop::OutputJob( parent ), m_workState(WorkIdle)
{
    setCapabilities(Killable);
    KDevelop::ICore::self()->uiController()->registerStatus(this);
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
    QString pattern = substitudePattern(m_templateString, m_patternString, m_regexpFlag);
    m_regExp.setPattern(pattern);
    m_regExp.setCaseSensitivity( m_caseSensitiveFlag ? Qt::CaseSensitive : Qt::CaseInsensitive );
    if(m_regExp.pattern() == QRegExp::escape(m_regExp.pattern())) {
        // this is obviously not a regexp, so do a regular search
        m_regExpSimple = m_regExp.pattern();
        m_regexpFlag = false;
    }
    GrepOutputModel *model = new GrepOutputModel();
    model->setRegExp(m_regExp);
    setModel(model, KDevelop::IOutputView::TakeOwnership);
    setDelegate(GrepOutputDelegate::self());
    startOutput();
    if(m_fileList.length()<100)
        emit showMessage(this, i18n("Searching for \"%1\"", m_regExp.pattern()));
    else
        emit showMessage(this, i18n("Searching for \"%1\" in %2 files", m_regExp.pattern(), m_fileList.length()));
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
                // this increases the speed
                for(int i=0; i<10; i++)
                {
                    if(m_fileIndex >= m_fileList.length())
                        break;
                    GrepOutputItem::List items;
                    if(m_regexpFlag)
                        items = grepFile(m_fileList[m_fileIndex].toLocalFile(), m_regExp);
                    else
                        items = grepFileSimple(m_fileList[m_fileIndex].toLocalFile(), m_regExpSimple, m_caseSensitiveFlag);
                    if(!items.isEmpty())
                        model()->appendOutputs(m_fileList[m_fileIndex].toLocalFile(), items);
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
    setToolTitle(i18n("Find in Files"));
    setToolIcon(KIcon("edit-find"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setTitle(m_patternString);
    setBehaviours( KDevelop::IOutputView::AllowUserClose );
    m_fileList.clear();
    m_workState = WorkIdle;
    m_fileIndex = 0;
    QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
}

QString GrepJob::substitudePattern(const QString& pattern, const QString& searchString, bool isRegexp)
{
    QString subst = searchString;
    if(!isRegexp)
        subst = QRegExp::escape(subst);
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

GrepOutputModel* GrepJob::model() const
{
    return static_cast<GrepOutputModel*>(OutputJob::model());
}

bool GrepJob::doKill()
{
    if(m_workState!=WorkIdle)
    {
        m_workState = WorkIdle;
        m_findThread->tryAbort();
        return m_findThread->isFinished();
    }
    return true;
}

void GrepJob::setTemplateString(const QString& templateString)
{
    m_templateString = templateString;
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

void GrepJob::setPatternString(const QString& patternString)
{
    m_patternString = patternString;

    setObjectName(i18n("Grep: %1", m_patternString));
}

#include "grepjob.moc"
