/***************************************************************************
                             makewidget.cpp
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


#include <qdir.h>
#include <qapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>

#include "outputviews.h"
#include "makewidget.h"


class MakeListBoxItem : public ProcessListBoxItem
{
public:
    MakeListBoxItem(const QString &s,
                    const QString &filename, int linenumber);
    QString filename()
        { return fn; }
    int linenumber()
        { return lineno; }
    virtual bool isCustomItem();

private:
    QString fn;
    int lineno;
};

MakeListBoxItem::MakeListBoxItem(const QString &s,
                                 const QString &filename,
                                 int linenumber)
    : ProcessListBoxItem(s, Error), fn(filename), lineno(linenumber)
{}


bool MakeListBoxItem::isCustomItem()
{
    return true;
}


MakeWidget::MakeWidget(MakeView *view)
    : ProcessView(0, "make widget")
{
    connect( this, SIGNAL(highlighted(int)),
             this, SLOT(lineHighlighted(int)) );

    m_part = view;
}


MakeWidget::~MakeWidget()
{}


void MakeWidget::startJob(const QString &dir, const QString &command)
{
    ProcessView::startJob(dir, command);
    dirstack.clear();
    dirstack.push(new QString(QDir::currentDirPath()));
}


void MakeWidget::nextError()
{
    // Search for a custom (= error) item beginning from selected
    // item or - if none is selected - from beginning
    int count = numRows();
    for (int i = currentItem()+1; i < count; ++i)
        if (static_cast<ProcessListBoxItem*>(item(i))->isCustomItem()) {
            setCurrentItem(i);
            return;
        }
            
    QApplication::beep();
}


void MakeWidget::prevError()
{
    // Search for a custom (= error) item beginning from selected
    // item or - if none is selected - from end
    int cur = (currentItem() == -1)? numRows() : currentItem();
    for (int i = cur; i >= 0; --i)
        if (static_cast<ProcessListBoxItem*>(item(i))->isCustomItem()) {
            setCurrentItem(i);
            return;
        }

    QApplication::beep();
}


void MakeWidget::childFinished(bool normal, int status)
{
    QString s;
    ProcessListBoxItem::Type t;
    
    if (normal) {
        if (status) {
            s = i18n("*** Exited with status: %1 ***").arg(status);
            t = ProcessListBoxItem::Error;
        } else {
            s = i18n("*** Success ***");
            t = ProcessListBoxItem::Diagnostic;
        }
    } else {
        s = i18n("*** Compilation aborted ***");
        t = ProcessListBoxItem::Error;
    }
    
    insertItem(new ProcessListBoxItem(s, t));
}


void MakeWidget::lineHighlighted(int line)
{
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item(line));
    if (i->isCustomItem()) {
        MakeListBoxItem *gi = static_cast<MakeListBoxItem*>(i);
        emit m_part->gotoSourceFile(gi->filename(), gi->linenumber());
    }
}


void MakeWidget::insertStdoutLine(const QString &line)
{
    // KRegExp has ERE syntax
    KRegExp enterDirRx("[^\n]*: Entering directory `([^\n]*)'$");
    KRegExp leaveDirRx("[^\n]*: Leaving directory `([^\n]*)'$");

    if (enterDirRx.match(line)) {
        QString *dir = new QString(enterDirRx.group(1));
        dirstack.push(dir);
        qDebug( "Entering dir: %s", (*dir).ascii() );
    }
    else if (leaveDirRx.match(line)) {
        kdDebug(9004) << "Leaving dir: " << leaveDirRx.group(1) << endl;
        QString *dir = dirstack.pop();
        kdDebug(9004) << "Now: " << (*dir) << endl;
        delete dir;
    }
    
    ProcessView::insertStdoutLine(line);
}


void MakeWidget::insertStderrLine(const QString &line)
{
    // KRegExp has ERE syntax
    KRegExp errorGccRx("([^: \t]+):([0-9]+):.*");
    KRegExp errorJadeRx("[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:.*");
    const int errorGccFileGroup = 1;
    const int errorJadeFileGroup = 1;
    const int errorGccRowGroup = 2;
    const int errorJadeRowGroup = 2;

    QString fn;
    int row;
    
    bool hasmatch = false;
    if (errorGccRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorGccFileGroup);
        row = QString(errorGccRx.group(errorGccRowGroup)).toInt()-1;
    } else if (errorJadeRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorJadeFileGroup);
        row = QString(errorJadeRx.group(errorJadeRowGroup)).toInt()-1;
    }
    
    if (hasmatch) {
        kdDebug(9004) << "Error in " << fn << " " << row << endl;
        if (dirstack.top())
            fn.prepend("/").prepend(*dirstack.top());
        kdDebug(9004) << "Path: " << fn << endl;
        insertItem(new MakeListBoxItem(line, fn, row));
    }
    else
        ProcessView::insertStderrLine(line);
}
