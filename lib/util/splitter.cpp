/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qframe.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qstyle.h>
#include <kdebug.h>

#include "splitter.h"


Splitter::Splitter(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    _or = Horizontal;

    handlelist.setAutoDelete(true);
}


Splitter::~Splitter()
{}


void Splitter::setOrientation(Orientation orient)
{
    if (orient != _or) {
        if (!childlist.isEmpty()) {
            if (hasMultipleChildren()) {
                kdDebug(9000) << "Cannot change the orientation of a splitter with several widgets." << endl;
                return;
            }
            if (orient == Horizontal) {
                SplitterHandle *handle = new SplitterHandle(childlist.first(), this, "handle");
                //                handle->show();
                handlelist.append(handle);
            } else {
                handlelist.remove((uint)0);
            }
        }

        _or = orient;
        doLayout();
    }
}


Qt::Orientation Splitter::orientation() const
{
    return _or;
}


bool Splitter::hasMultipleChildren() const
{
    return childlist.count() > 1;
}


QWidget *Splitter::firstChild()
{
    return childlist.first();
}


int Splitter::handleWidth() const
{
    return (orientation() == Horizontal)? style().pixelMetric(QStyle::PM_SplitterWidth, this) : 0;
}


void Splitter::addChild(QWidget *w)
{
    kdDebug(9000) << "add child" << endl;
    //    connect( w, SIGNAL(destroyed()), this, SLOT(childDestroyed()) );
    if (childlist.count()) {
        // When we already have children, this is the same as
        // a split of the last one
        splitChild(childlist.last(), w);
    } else {
        childlist.append(w);
        if (orientation() == Horizontal) {
            SplitterHandle *handle = new SplitterHandle(w, this, "handle");
            handlelist.append(handle);
        }
    }

    doLayout();
}


void Splitter::splitChild(QWidget *old, QWidget *w)
{
#if 0
    kdDebug(9000) << "splitChild - Old list of splitter children:" << endl;
    QPtrListIterator<QWidget> it1(childlist);
    for (; it1.current(); ++it1)
        kdDebug(9000) << it1.current()->name() << endl;
#endif

    int offset = 0;
    for (uint i = 0; i < childlist.count(); ++i) {
        if (childlist.at(i) == old) {
            childlist.insert(i+1, w);
            if (orientation() == Horizontal) {
                SplitterHandle *handle = new SplitterHandle(w, this, "handle");
                handlelist.insert(i+1, handle);
            }
            if (i+1 != childlist.count()-1) {
                sizes.insert(sizes.at(i), sizes[i]/2 - handleWidth());
                sizes[i+1] = sizes[i]/2;
            } else {
                int total = (orientation() == Horizontal)? width() : height();
                sizes.insert(sizes.at(i), (total-offset)/2);
            }
            doLayout();
            return;
        }
        offset += sizes[i];
        offset += handleWidth();
    }

    kdDebug(9000) << "Huh? Old child not in splitter" << endl;
}


void Splitter::replaceChild(QWidget *old, QWidget *w)
{
    for (uint i=0; i < childlist.count(); ++i)
        if (childlist.at(i) == old) {
            childlist.remove(i);
            if (orientation() == Horizontal)
                handlelist.remove(i);
            childlist.insert(i, w);
            if (orientation() == Horizontal) {
                SplitterHandle *handle = new SplitterHandle(w, this, "handle");
                handlelist.insert(i, handle);
            }
            doLayout();
            return;
        }

    kdDebug(9000) << "Huh? Old child not in splitter" << endl;
}


void Splitter::moveSplitter(QWidget *w, int pos)
{
    int offset = 0;
    for (uint i=0; i < childlist.count(); ++i) {
        if (childlist.at(i) == w) {
            if (i != childlist.count()) {
                sizes[i] = pos-offset;
                break;
            }
        }
        offset += sizes[i];
        offset += handleWidth();
    }

    doLayout();
}


void Splitter::doLayout()
{
    setUpdatesEnabled(false);

    if (orientation() == Horizontal) {

        int offset = 0;
        uint i;
        for (i=0; i+1 < childlist.count(); ++i) {
            sizes[i] = QMAX(sizes[i], childlist.at(i)->minimumSizeHint().width());
            childlist.at(i)->setGeometry(offset, 0, sizes[i], height());
            offset += childlist.at(i)->width();
            handlelist.at(i)->setGeometry(offset, 0, style().pixelMetric(QStyle::PM_SplitterWidth, this), height());
            handlelist.at(i)->show();
            offset += handlelist.at(i)->width();
        }

        if (childlist.count()) {
            childlist.at(i)->setGeometry(offset, 0, width()-offset, height());
            handlelist.at(i)->hide();
        }

    } else {

        int offset = 0;
        uint i;
        for (i=0; i+1 < childlist.count(); ++i) {
            sizes[i] = QMAX(sizes[i], childlist.at(i)->minimumSizeHint().height());
            childlist.at(i)->setGeometry(0, offset, width(), sizes[i]);
            offset += childlist.at(i)->height();
        }

        if (childlist.count())
            childlist.at(i)->setGeometry(0, offset, width(), height()-offset);

    }

    setUpdatesEnabled(true);
}



QSize Splitter::minimumSizeHint() const
{
    int extent = 0;
    for (uint i = 0; i < childlist.count(); ++i) {
        // Bah.
        Splitter *that = (Splitter*) this;
        QSize s = that->childlist.at(i)->minimumSizeHint();
        extent = QMAX(extent, (orientation()==Horizontal)? s.height() : s.width());
    }
    return (orientation()==Horizontal)? QSize(0, extent) : QSize(extent, 0);
}


void Splitter::childEvent(QChildEvent *e)
{
    if (e->type() != QEvent::ChildRemoved)
        return;

    QObject *o = e->child();

    for (uint i=0; i<childlist.count(); ++i) {
        if (childlist.at(i) == o) {
            childlist.remove(i);
            if (orientation() == Horizontal)
                handlelist.remove(i);
            if (i == childlist.count())
                sizes.remove(i-1);
            else
                sizes.remove(i);
            doLayout();
            if (childlist.count() < 2)
                QTimer::singleShot(0, this, SLOT(collapse()));
            break;
        }
    }
}


void Splitter::collapse()
{
    emit collapsed(this);
}


void Splitter::resizeEvent(QResizeEvent */*e*/)
{
    doLayout();
}


SplitterBar::SplitterBar(QWidget *parent, const char *name)
    : QFrame(parent, name)
{
    setFrameStyle(Panel | Raised);
    setLineWidth(1);
    setFixedHeight(fontMetrics().lineSpacing()+2);

    offset = 0;
}


SplitterBar::~SplitterBar()
{}


void SplitterBar::mouseMoveEvent(QMouseEvent *e)
{
    if (e->state() & LeftButton) {
        QWidget *p = parentWidget();
        QWidget *gp = p->parentWidget();
        if (!gp->inherits("Splitter"))
            return;
        // If we are in a horizontal splitter, look if this horizontal
        // splitter is nested in a vertical splitter, when is then
        // used.
        if (static_cast<Splitter*>(gp)->orientation() == Horizontal) {
            p = gp;
            gp = p->parentWidget();
            if (!gp->inherits("Splitter"))
                return;
        }
        int pos = gp->mapFromGlobal(e->globalPos()).y() + offset;
	static_cast<Splitter*>(gp)->moveSplitter(p, pos);
    }
}


void SplitterBar::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() & LeftButton )
	offset = height()-e->pos().y();
}


void SplitterBar::childEvent(QChildEvent *e)
{
    if (e->type() == QEvent::ChildInserted && e->child()->isWidgetType())
        e->child()->installEventFilter(this);
    else if (e->type() == QEvent::ChildRemoved && e->child()->isWidgetType())
        e->child()->removeEventFilter(this);
}


bool SplitterBar::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
        mousePressEvent(static_cast<QMouseEvent*>(e));
    else if(e->type() == QEvent::MouseMove)
        mouseMoveEvent(static_cast<QMouseEvent*>(e));

    return QWidget::eventFilter(o, e);
}


SplitterHandle::SplitterHandle(QWidget *embeddedSibling, Splitter *parent, const char *name)
    : QWidget(parent, name)
{
    sibling = embeddedSibling;
    setCursor(splitHCursor);
    //    setFixedWidth(QMIN(style().splitterWidth(), QApplication::globalStrut().width()));
}


SplitterHandle::~SplitterHandle()
{}


void SplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (e->state() & LeftButton) {
        int pos = parentWidget()->mapFromGlobal(e->globalPos()).x() + offset;
	static_cast<Splitter*>(parentWidget())->moveSplitter(sibling, pos);
    }
}


void SplitterHandle::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() & LeftButton )
	offset = -e->pos().x();
}


void SplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    style().drawPrimitive(QStyle::PE_Splitter, &p, QRect(0, 0, width(), height()), colorGroup(), QStyle::Style_Horizontal);
}

#include "splitter.moc"
