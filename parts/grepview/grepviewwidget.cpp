/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qpainter.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kprocess.h>
#include <kparts/part.h>
#include <ktexteditor/selectioninterface.h>
using namespace KTextEditor;

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"

#include "grepdlg.h"
#include "grepviewpart.h"
#include "grepviewwidget.h"


class GrepListBoxItem : public ProcessListBoxItem
{
public:
    GrepListBoxItem(const QString &fileName, const QString &lineNumber, const QString &text, bool showFilename);
    QString filename()
        { return fileName; }
    int linenumber()
        { return lineNumber.toInt(); }
    virtual bool isCustomItem();

private:
    virtual void paint(QPainter *p);
    QString fileName, lineNumber, text;
    bool show;
};


GrepListBoxItem::GrepListBoxItem(const QString &fileName, const QString &lineNumber, const QString &text, bool showFilename)
    : ProcessListBoxItem( QString::null, Normal), 
	fileName(fileName), lineNumber(lineNumber), text(text.stripWhiteSpace()),
	show(showFilename)
{}


bool GrepListBoxItem::isCustomItem()
{
    return true;
}


void GrepListBoxItem::paint(QPainter *p)
{
    QFontMetrics fm = p->fontMetrics();
    QString stx = lineNumber + ":  ";
    int y = fm.ascent()+fm.leading()/2;
    int x = 3;
	if (show)
	{
		p->setPen(Qt::darkGreen);
		p->drawText(x, y, fileName);
		x += fm.width(fileName);
    }
    else {
    p->setPen(Qt::black);
    QFont font1(p->font());
    QFont font2(font1);
    font2.setBold(true);
    p->setFont(font2);
    p->drawText(x, y, stx);
    p->setFont(font1);
    x += fm.width(stx);

    p->setPen(Qt::blue);
    p->drawText(x, y, text);
	}
}


GrepViewWidget::GrepViewWidget(GrepViewPart *part)
    : ProcessWidget(0, "grep widget")
    , m_matchCount(0)
{
    grepdlg = new GrepDialog(this, "grep widget");
    connect( grepdlg, SIGNAL(searchClicked()),
	     this, SLOT(searchActivated()) );
    connect( this, SIGNAL(clicked(QListBoxItem*)),
             this, SLOT(slotExecuted(QListBoxItem*)) );
    connect( this, SIGNAL(returnPressed(QListBoxItem*)),
             this, SLOT(slotExecuted(QListBoxItem*)) );

    m_part = part;
}


GrepViewWidget::~GrepViewWidget()
{}


void GrepViewWidget::showDialog()
{
    // Get the selected text if there is any
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(m_part->partController()->activePart());
    if (ro_part)
    {
        SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
        if(selectIface && selectIface->hasSelection())
        {
            QString selText = selectIface->selection();
            if(!selText.contains('\n'))
            {
                grepdlg->setPattern(selText);
            }
        }
    }
    grepdlg->show();
}

// @todo - put this somewhere common - or just use Qt if possible
static QString escape(const QString &str)
{
    QString escaped("[]{}()\\^$?.+-*");
    QString res;

    for (uint i=0; i < str.length(); ++i) {
        if (escaped.find(str[i]) != -1)
            res += "\\";
        res += str[i];
    }

    return res;
}


void GrepViewWidget::showDialogWithPattern(QString pattern)
{
    // Before anything, this removes line feeds from the
    // beginning and the end.
    int len = pattern.length();
    if (len > 0 && pattern[0] == '\n')
	{
	    pattern.remove(0, 1);
	    len--;
	}
    if (len > 0 && pattern[len-1] == '\n')
	pattern.truncate(len-1);
    grepdlg->setPattern( pattern );
    grepdlg->show();
}


void GrepViewWidget::searchActivated()
{
    m_matchCount = 0;
    _lastfilename = "";

    QString files;
    // waba: code below breaks on filenames containing a ',' !!!
    QStringList filelist = QStringList::split(",", grepdlg->filesString());
    if (!filelist.isEmpty())
        {
            QStringList::Iterator it(filelist.begin());
            files = KShellProcess::quote(*it);
            ++it;
            for (; it != filelist.end(); ++it)
                files += " -o -name " + KShellProcess::quote(*it);
        }

    QString pattern = grepdlg->templateString();
//    pattern.replace(QRegExp("%s"), grepdlg->patternString());
    pattern.replace(QRegExp("%s"), escape( grepdlg->patternString() ) );
    pattern.replace(QRegExp("'"), "'\\''");

    QString filepattern = "find ";
    filepattern += KShellProcess::quote(grepdlg->directoryString());
    if (!grepdlg->recursiveFlag())
        filepattern += " -maxdepth 1";
    filepattern += " \\( -name ";
    filepattern += files;
    filepattern += " \\) -print -follow";

    QString command = filepattern + " " ;
    if (grepdlg->ignoreSCMDirsFlag()) {
        command += "| grep -v \"SCCS/\" ";
        command += "| grep -v \"CVS/\" ";
    }

    // quote spaces in filenames going to xargs 
    command += "| sed \"s/ /\\\\\\ /g\" "; 

    command += "| xargs " ;

#ifndef USE_SOLARIS
    command += "egrep -H -n -e ";
#else
    // -H reported as not being available on Solaris,
    // but we're buggy without it on Linux.
    command += "egrep -n -e ";
#endif
    command += KShellProcess::quote(pattern);
    startJob("", command);

    m_part->mainWindow()->raiseView(this);
    m_part->core()->running(m_part, true);
}


void GrepViewWidget::childFinished(bool normal, int status)
{
    // When xargs executes grep several times (because the command line
    // generated would be too large for a single grep) and one of those
    // sets of files passed to grep does not contain a match, then an
    // error status of 123 is set by xargs, even if there is a match in
    // another set of files.
    // Reset this false status here.
    if (status == 123 && numRows() > 1)
        status = 0;

    insertItem(new ProcessListBoxItem(i18n("*** %n match found. ***", "*** %n matches found. ***", m_matchCount), ProcessListBoxItem::Diagnostic));
    maybeScrollToBottom();

    ProcessWidget::childFinished(normal, status);
    m_part->core()->running(m_part, false);
}


void GrepViewWidget::slotExecuted(QListBoxItem* item)
{
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item);
    if (!i || !i->isCustomItem())
        return;

    GrepListBoxItem *gi = static_cast<GrepListBoxItem*>(i);
    m_part->partController()->editDocument( KURL( gi->filename() ), gi->linenumber()-1 );
//    m_part->mainWindow()->lowerView(this);
}


void GrepViewWidget::insertStdoutLine(const QString &line)
{
    int pos;
    QString filename, linenumber, rest;

    QString str = line;
    if ( (pos = str.find(':')) != -1)
        {
            filename = str.left(pos);
            str.remove( 0, pos+1 );
            if ( ( pos = str.find(':') ) != -1)
                {
                    linenumber = str.left(pos);
                    str.remove( 0, pos+1 );
                    // filename will be displayed only once
                    // selecting filename will display line 1 of file,
                    // otherwise, line of requested search
                    if ( _lastfilename != filename )
                    {
                        _lastfilename = filename;
                        insertItem(new GrepListBoxItem(filename, "0", str, true));
                        insertItem(new GrepListBoxItem(filename, linenumber, str, false));
                    }
                    else 
                    {
                        insertItem(new GrepListBoxItem(filename, linenumber, str, false));
                    }
                    maybeScrollToBottom();
                }
            m_matchCount++;
        }
}


void GrepViewWidget::projectChanged(KDevProject *project)
{
    QString dir = project? project->projectDirectory() : QDir::homeDirPath();
    grepdlg->setDirectory(dir);
}

#include "grepviewwidget.moc"
