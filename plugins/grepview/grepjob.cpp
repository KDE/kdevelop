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
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <project/projectmodel.h>
#include <util/processlinemaker.h>
#include <ktexteditor/document.h>

#include <language/duchain/indexedstring.h>
#include <interfaces/iuicontroller.h>

#include "grepoutputdelegate.h"

using namespace KDevelop;

static GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re)
{
    GrepOutputItem::List res;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return res;
    int lineno = 1;
    while( !file.atEnd() )
    {
        QByteArray data = file.readLine();
        if( re.indexIn(data)!=-1 )
            res << GrepOutputItem(filename, lineno, QString(data).trimmed());
        lineno++;
    }
    file.close();
    return res;
}

static GrepOutputItem::List replaceFile(const QString &filename, const QRegExp &re, const QString &repl)
{
    GrepOutputItem::List res;
    KUrl url(filename);
    
    IDocument* doc = ICore::self()->documentController()->documentForUrl( url );
    bool alreadyOpened = true;
    if(!doc)
    {
        doc = ICore::self()->documentController()->openDocument( url, 
                                                                 KTextEditor::Range::invalid(), 
                                                                 IDocumentController::DoNotActivate);
        alreadyOpened = false;
    }
    if(!doc)
        return res;
    
    KTextEditor::Document* textDoc = doc->textDocument();
    if(!textDoc)
        return res;
    
    textDoc->startEditing();
    for(int lineno = 0; lineno < textDoc->lines(); lineno++)
    {
        QString line = textDoc->line(lineno);
        if( re.indexIn(line)!=-1 )
        {
            textDoc->removeLine(lineno);
            textDoc->insertLine(lineno, line.replace(re, repl));
            res << GrepOutputItem(filename, lineno+1, line.trimmed());
        }
    }
    textDoc->endEditing();
    
    if(!alreadyOpened && res.isEmpty())
        doc->close();
    
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
    static_cast<GrepOutputModel*>(model())->setRegExp(m_regExp);
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
                if(m_fileIndex < m_fileList.length()) {
                    QString file = m_fileList[m_fileIndex].toLocalFile();
                    GrepOutputItem::List items = m_replaceFlag ? replaceFile(file, m_regExp, m_replaceString) : grepFile(file, m_regExp);

                    if(!items.isEmpty())
                        emit foundMatches(file, items);

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

    GrepOutputModel *model = new GrepOutputModel(this);
    setModel(model, KDevelop::IOutputView::TakeOwnership);
    setDelegate(GrepOutputDelegate::self());
    startOutput();

    connect(this, SIGNAL(showErrorMessage(QString, int)),
            model, SLOT(showErrorMessage(QString)));
    connect(this, SIGNAL(showMessage(KDevelop::IStatus*, QString, int)),
            model, SLOT(showMessage(KDevelop::IStatus*, QString)));

    qRegisterMetaType<GrepOutputItem::List>();
    connect(this, SIGNAL(foundMatches(QString, GrepOutputItem::List)),
            model, SLOT(appendOutputs(QString, GrepOutputItem::List)), Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, "slotWork", Qt::QueuedConnection);
}

GrepOutputModel* GrepJob::model() const
{
    return static_cast<GrepOutputModel*>(OutputJob::model());
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
