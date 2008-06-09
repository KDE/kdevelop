/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepviewplugin.h"
#include "grepdialog.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"

#include <QWhatsThis>
#include <QList>
#include <QRegExp>
#include <QKeySequence>

#include <kprocess.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <ioutputview.h>
#include <iplugincontroller.h>
#include <iuicontroller.h>
#include <kparts/mainwindow.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <projectmodel.h>
#include <processlinemaker.h>

#include "grepjob.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(GrepViewFactory, registerPlugin<GrepViewPlugin>(); )
K_EXPORT_PLUGIN(GrepViewFactory("kdevgrepview"))

GrepViewPlugin::GrepViewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( GrepViewFactory::componentData(), parent )
    , m_projectForActiveFile(0), m_delegate(new GrepOutputDelegate(this))
{
    setXMLFile("kdevgrepview.rc");

    m_grepdlg = new GrepDialog( this );
    connect( m_grepdlg, SIGNAL(search()), this, SLOT(searchActivated()) );

    KAction *action = actionCollection()->addAction("edit_grep");
    action->setText(i18n("Find in Fi&les..."));
    action->setShortcut( QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F) );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotGrep()));
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("<b>Find in files</b><p>"
            "Opens the 'Find in files' dialog. There you "
            "can enter a regular expression which is then "
            "searched for within all files in the directories "
            "you specify. Matches will be displayed, you "
            "can switch to a match directly.</p>") );
}

GrepViewPlugin::~GrepViewPlugin()
{
    delete m_grepdlg;
}

void GrepViewPlugin::slotGrep()
{
    QString contextString = currentSelectedWord();
    if( contextString.isEmpty() )
        contextString = currentWord();
    showDialogWithPattern( contextString );

}

void GrepViewPlugin::showDialogWithPattern(const QString& p)
{
    // Before anything, this removes line feeds from the
    // beginning and the end.
    QString pattern = p;
    int len = pattern.length();
    if (len > 0 && pattern[0] == '\n')
    {
        pattern.remove(0, 1);
        len--;
    }
    if (len > 0 && pattern[len-1] == '\n')
        pattern.truncate(len-1);
    m_grepdlg->setPattern( pattern );

    m_grepdlg->enableButtonOk( !pattern.isEmpty() );

    KUrl currentUrl;
    KDevelop::IDocument *document = core()->documentController()->activeDocument();
    if( document )
    {
        currentUrl = document->url();
    }
    if( currentUrl.isValid() )
    {
        KDevelop::IProject *proj =
                core()->projectController()->findProjectForUrl( currentUrl );
        if( proj )
        {
            m_grepdlg->setEnableProjectBox(! proj->files().isEmpty() );
            m_projectForActiveFile = proj;
        }
        else
        {
            m_grepdlg->setEnableProjectBox(false);
        }
    }

    m_grepdlg->show();
}

void GrepViewPlugin::searchActivated()
{
    GrepJob* job = new GrepJob(this);
    
    job->patternString = m_grepdlg->patternString();
    job->templateString = m_grepdlg->templateString();
    job->filesString = m_grepdlg->filesString();
    job->excludeString = m_grepdlg->excludeString();
    job->directory = m_grepdlg->directory();

    job->useProjectFilesFlag = m_grepdlg->useProjectFilesFlag();
    job->regexpFlag = m_grepdlg->regexpFlag();
    job->recursiveFlag = m_grepdlg->recursiveFlag();
    job->noFindErrorsFlag = m_grepdlg->noFindErrorsFlag();
    job->caseSensitiveFlag = m_grepdlg->caseSensitiveFlag();

    job->project = m_projectForActiveFile;
    
    ICore::self()->runController()->registerJob(job);
}

// from kdeveditorutil.cpp in kdev3.4, plus porting
QString GrepViewPlugin::currentWord()
{
    KDevelop::IDocument *doc =
             core()->documentController()->activeDocument();

    if( !doc ) return QString();

    KTextEditor::Document* ktedoc = doc->textDocument();

    if( !ktedoc ) return QString();

    KTextEditor::View *view = ktedoc->activeView();
    if( !view ) return QString();

    KTextEditor::Cursor cursor = view->cursorPositionVirtual();
    int line = cursor.line();
    int col  = cursor.column();
    QString linestr = ktedoc->line(line);

    int startPos = qMax( qMin( col, linestr.length()-1 ), 0 );
    int endPos = startPos;
    while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' || linestr[startPos] == '~') )
        startPos--;
    while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
        endPos++;

    return ( ( startPos == endPos ) ? QString() : linestr.mid( startPos+1, endPos-startPos-1 ) );
}

QString GrepViewPlugin::currentSelectedWord()
{
    KDevelop::IDocument *doc =
             core()->documentController()->activeDocument();

    if( !doc ) return QString();

    KTextEditor::Document* ktedoc = doc->textDocument();

    if( !ktedoc ) return QString();

    KTextEditor::View *view = ktedoc->activeView();
    if( !view ) return QString();
    return view->selectionText();
}

GrepOutputDelegate* GrepViewPlugin::delegate() const
{
    return m_delegate;
}

#include "grepviewplugin.moc"
