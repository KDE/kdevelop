/***************************************************************************
                             grepview.cpp
                             -------------------                                         

    copyright            : (C) 1999 The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "cproject.h"
#include "ctoolclass.h"
#include "grepview.h"
#include "grepdlg.h"
#include "misc.h"
#include "qregexp.h"

class GrepListBoxItem : public QListBoxText
{
public:
    GrepListBoxItem(const QString &s1, const QString &s2, const QString &s3);
    QString filename()
        { return str1; }
    int linenumber()
        { return str2.right(str2.length()-1).toInt()-1; }
    virtual void paint(QPainter *p);

private:
    QString str1, str2, str3;
};


GrepListBoxItem::GrepListBoxItem(const QString &s1,
                                 const QString &s2,
                                 const QString &s3)
{
    str1 = s1;
    str2 = s2;
    str3 = s3;
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


GrepView::GrepView(QWidget *parent, const char *name)
    : ProcessView(parent, name)
{
    grepdlg = new GrepDialog(this, "grep dialog");
    connect( grepdlg, SIGNAL(searchClicked()),
	     this, SLOT(searchActivated()) );
    connect( this, SIGNAL(highlighted(int)),
             this, SLOT(lineHighlighted(int)) );
}


GrepView::~GrepView()
{}


void GrepView::showDialog()
{
    grepdlg->show();
}


void GrepView::showDialogWithPattern(QString pattern)
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
    grepdlg->setPattern(CToolClass::escapetext(pattern, true));
    grepdlg->show();
}


void GrepView::searchActivated()
{
    QString files;
    QStringList filelist = QStringListsplit(",", grepdlg->filesString());
    if (!filelist.isEmpty())
        {
            QStringList::Iterator it(filelist.begin());
            files = "'" + (*it) + "'";
            ++it;
            for (; it != filelist.end(); ++it)
                files += " -o -name '" + (*it) + "'";
        }

    QString pattern = grepdlg->templateString();
    pattern.replace(QRegExp("%s"), grepdlg->patternString());
    pattern.replace(QRegExp("'"), "'\\''");

    QString filepattern = "`find '";
    filepattern += grepdlg->directoryString();
    filepattern += "'";
    if (!grepdlg->recursiveFlag())
        filepattern += " -maxdepth 1";
    filepattern += " -name ";
    filepattern += files;
    filepattern += "`";

    prepareJob("");
    (*this) << "grep";
    (*this) << "-n";
    (*this) << (QString("-e '") + pattern + "'");
    (*this) << filepattern;
    (*this) << "/dev/null";
    startJob();
}


#include <iostream.h>
void GrepView::lineHighlighted(int line)
{
    GrepListBoxItem *i = static_cast<GrepListBoxItem*>(item(line));
    emit itemSelected(i->filename(), i->linenumber());
    cout << "sel: " << i->filename() << "," << i->linenumber() << endl;
}


void GrepView::insert(const QString &line)
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
                    //emit itemSelected(filename,linenumber.toInt()-1);
                }
        }
}


void GrepView::projectOpened(CProject *prj)
{
    grepdlg->setDirectory(prj->getProjectDir());
}


void GrepView::compilationAborted()
{
    killJob();
}
