/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qregexp.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevcore.h"
#include "cvswidget.h"
#include "cvswidget.moc"
#include "cvspart.h"

class CvsListBoxItem : public ProcessListBoxItem {
public:
    CvsListBoxItem(const QString &s1);
    QString fileName() {return filename;}
    bool containsFileName() {return !filename.isEmpty();}
    virtual bool isCustomItem();

private:
    virtual void paint(QPainter *p);
    QString str1;
    QString filename;
};


CvsListBoxItem::CvsListBoxItem(const QString &s1)
        : ProcessListBoxItem(s1, Normal) {
    str1 = s1;
    QRegExp re("[ACMPRU?] (.*)");
    if (re.exactMatch(s1)) filename = re.cap(1);
}


bool CvsListBoxItem::isCustomItem() {
    return true;
}


void CvsListBoxItem::paint(QPainter *p) {
    QFontMetrics fm = p->fontMetrics();
    int y = fm.ascent()+fm.leading()/2;
    int x = 3;

    p->setPen(Qt::darkGreen);
    p->drawText(x, y, str1);
}

CvsWidget::CvsWidget(CvsPart *part)
        : ProcessWidget(0, "cvs widget") {
    connect( this, SIGNAL(highlighted(int)),
             this, SLOT(lineHighlighted(int)) );

    m_part = part;
}



CvsWidget::~CvsWidget() {}


void CvsWidget::startCommand(const QString &dir, const QString &command) {
    m_part->mainWindow()->raiseView(this);
    m_part->core()->running(m_part, true);
    this->dir = dir;
    startJob(dir,command);
}

void CvsWidget::lineHighlighted(int line) {
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item(line));
    if (i->isCustomItem()) {
        CvsListBoxItem *ci = static_cast<CvsListBoxItem*>(i);
    if (ci->containsFileName()) {
        m_part->partController()->editDocument(dir + "/" + ci->fileName());
        m_part->mainWindow()->lowerView(this);
    }
    }
}

void CvsWidget::insertStdoutLine(const QString & line) {
    insertItem(new CvsListBoxItem(line));
}

void CvsWidget::childFinished(bool normal, int status)
{
    m_part->core()->running( m_part, false );

    ProcessWidget::childFinished( normal, status );
}
