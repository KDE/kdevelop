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
    GrepListBoxItem(const QString &s1, const QString &s2, const QString &s3);
    QString filename()
        { return str1; }
    int linenumber()
        { return str2.right(str2.length()-1).toInt()-1; }
    virtual bool isCustomItem();

private:
    virtual void paint(QPainter *p);
    QString str1, str2, str3;
};


GrepListBoxItem::GrepListBoxItem(const QString &s1,
                                 const QString &s2,
                                 const QString &s3)
    : ProcessListBoxItem(s1+s2+s3, Normal)
{
    str1 = s1;
    str2 = s2;
    str3 = s3;
}


bool GrepListBoxItem::isCustomItem()
{
    return true;
}


void GrepListBoxItem::paint(QPainter *p)
{
    QFontMetrics fm = p->fontMetrics();
    int y = fm.ascent()+fm.leading()/2;
    int x = 3; 

    p->setPen(Qt::darkGreen);
    p->drawText(x, y, str1);
    x += fm.width(str1);
    
    p->setPen(Qt::black);
    QFont font1(p->font());
    QFont font2(font1); 
    font2.setBold(true);
    p->setFont(font2);
    p->drawText(x, y, str2);
    p->setFont(font1);
    x += fm.width(str2);
    
    p->setPen(Qt::blue);
    p->drawText(x, y, str3);
}


GrepViewWidget::GrepViewWidget(GrepViewPart *part)
    : ProcessWidget(0, "grep widget")
{
    grepdlg = new GrepDialog(this, "grep widget");
    connect( grepdlg, SIGNAL(searchClicked()),
	     this, SLOT(searchActivated()) );
    connect( this, SIGNAL(highlighted(int)),
             this, SLOT(lineHighlighted(int)) );

    m_part = part;
}


GrepViewWidget::~GrepViewWidget()
{}


void GrepViewWidget::showDialog()
{
    grepdlg->show();
}


static QString escape(const QString &str)
{
    QString escaped("[]\\^$");
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
    // Then special chars are escaped.
    grepdlg->setPattern(escape(pattern));
    grepdlg->show();
}


void GrepViewWidget::searchActivated()
{
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
    pattern.replace(QRegExp("%s"), grepdlg->patternString());
    pattern.replace(QRegExp("'"), "'\\''");

    QString filepattern = "find ";
    filepattern += KShellProcess::quote(grepdlg->directoryString());
    if (!grepdlg->recursiveFlag())
        filepattern += " -maxdepth 1";
    filepattern += " \\( -name ";
    filepattern += files;
    filepattern += " \\) -print";

    QString command = filepattern + " | xargs " ;
    command += "egrep -n -e ";
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

    insertItem(new ProcessListBoxItem(i18n("*** %1 match found. ***", "*** %1 matches found. ***", numRows() - 1).arg(numRows() - 1), ProcessListBoxItem::Diagnostic));

    ProcessWidget::childFinished(normal, status);
    m_part->core()->running(m_part, false);
}


void GrepViewWidget::lineHighlighted(int line)
{
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item(line));
    if (i->isCustomItem())
        {
            GrepListBoxItem *gi = static_cast<GrepListBoxItem*>(i);
            m_part->partController()->editDocument(gi->filename(), gi->linenumber());
            m_part->mainWindow()->lowerView(this);
        }
}


void GrepViewWidget::insertStdoutLine(const QString &line)
{
    int pos;
    QString filename, linenumber, rest;

    QString str = line;
    if ( (pos = str.find(':')) != -1)
        {
            filename = str.left(pos);
            str.remove(0, pos);
            if ( (pos = str.find(':', 1)) != -1)
                {
                    linenumber = str.left(pos);
                    str.remove(0, pos);
                    insertItem(new GrepListBoxItem(filename, linenumber, str));
                }
        }
}


void GrepViewWidget::projectChanged(KDevProject *project)
{
    QString dir = project? project->projectDirectory() : QDir::homeDirPath();
    grepdlg->setDirectory(dir);
}

#include "grepviewwidget.moc"
