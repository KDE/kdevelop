/***************************************************************************
                             kdlgproplv.cpp
                             -------------------
    begin                : Wed Mar 17 1999
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qframe.h>
#include <qstringlist.h>
#include <kapp.h>
#include <kconfig.h>
#include "kdlgedit.h"
#include "kdlgitembase.h"
#include "kdlgpropertybase.h"
#include "kdlgproplv.h"
#include "kdlgproplvis.h"


AdvListView::AdvListView(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    KConfig *config = kapp->config();
    config->setGroup("KDlgEdit");
    openStats = config->readListEntry("PropOpenStats");
}


AdvListView::~AdvListView()
{
  saveOpenStats();

  KConfig *config = kapp->config();
  config->setGroup("KDlgEdit");
  config->writeEntry("PropOpenStats", openStats);
}


void AdvListView::viewportMousePressEvent(QMouseEvent *e)
{
    QListView::viewportMousePressEvent( e );
    updateWidgets();
}


void AdvListView::paintEvent(QPaintEvent *e)
{
    QListView::paintEvent( e );
    updateWidgets();
}


void AdvListView::mouseMoveEvent(QMouseEvent *e)
{
    QListView::mousePressEvent( e );
    updateWidgets();
}


void AdvListView::keyPressEvent(QKeyEvent *e)
{
    QListView::keyPressEvent( e );
    updateWidgets();
}


void AdvListView::moveEvent(QMoveEvent *e)
{
    QListView::moveEvent( e );
    updateWidgets();
}


void AdvListView::resizeEvent( QResizeEvent *e)
{
    QListView::resizeEvent( e );
    updateWidgets();
}


void AdvListView::hideAll()
{
    AdvListViewItem *i = (AdvListViewItem*)firstChild();

    while (i) {
        i->hideWidgets();
        i = (AdvListViewItem*)i->nextSibling();
    }
}


void AdvListView::updateWidgets()
{
    AdvListViewItem* i = (AdvListViewItem*)firstChild();

    while (i) {
        i->updateWidgets();
        i = (AdvListViewItem*)i->nextSibling();
    }
}


void AdvListView::setGeometryEntries(int x, int y, int w, int h)
{
    AdvListViewItem *i = (AdvListViewItem*)firstChild();
    
    while (i) {
        AdvListViewItem *n = (AdvListViewItem*)i->firstChild();
        while (n) {
            if (n->text(0) == "X")
                n->setText(1, QString().setNum(x));
            if (n->text(0) == "Y")
                n->setText(1, QString().setNum(y));
            if (n->text(0) == "Width")
                n->setText(1, QString().setNum(w));
            if (n->text(0) == "Height")
                n->setText(1, QString().setNum(h));
            n = (AdvListViewItem*)n->nextSibling();
        }
        
        i = (AdvListViewItem*)i->nextSibling();
    }
}


void AdvListView::saveOpenStats()
{
    openStats.clear();
    
    AdvListViewItem *it = (AdvListViewItem*)firstChild();

    while (it) {
        if (it->isOpen())
            openStats.insert(openStats.end(), it->text(0));
        it = (AdvListViewItem*)it->nextSibling();
    }
}


void AdvListView::restoreOpenStats()
{
    AdvListViewItem* it = (AdvListViewItem*)firstChild();

    while (it) {
        if (openStats.contains(it->text(0)))
            it->setOpen(true);
        
        it = (AdvListViewItem*)it->nextSibling();
    }
}


AdvListViewItem::AdvListViewItem(QListView *parent, QString a, QString b)
    : QListViewItem( parent, a, b ), colwid(0)
{}


AdvListViewItem::AdvListViewItem(AdvListViewItem *parent, QString a, QString b)
   : QListViewItem( parent, a, b ), colwid(0)
{}


AdvListViewItem::~AdvListViewItem()
{
    delete colwid;
}


void AdvListViewItem::setColumnWidget(AdvLvi_Base *wp)
{
    colwid = wp;
}


void AdvListViewItem::hideWidgets()
{
    if (colwid)
        colwid->hide();

    AdvListViewItem *it = (AdvListViewItem*)firstChild();

    while (it) {
        it->hideWidgets();
        it = (AdvListViewItem*)it->nextSibling();
    }
}


void AdvListViewItem::updateWidgets()
{
    testAndResizeWidget();
    
    AdvListViewItem* i = (AdvListViewItem*)firstChild();

    while (i) {
        i->updateWidgets();
        i = (AdvListViewItem*)i->nextSibling();
    }
}



void AdvListViewItem::testAndResizeWidget()
{
    // Widget activated?
    if (!colwid)
        return;
    
#if defined(LVDEBUG)
    debug("testAndResizeWidget %s, (selected=%d)", text(column), isSelected());
#endif
    
    // if the line the class  builds is selected and the
    // widget is active we can show it, otherwise we have
    // to hide and take the focus away from it.
    if (isSelected() && ((!parent()) || (parent()->isOpen()))) {
        colwid->show();
        listView()->ensureItemVisible(this);
    } else {
        colwid->hide();
            
        if (colwid->getDisplayedText().isNull())
            setText(1, colwid->getText());
        else
            setText(1, colwid->getDisplayedText());
           
        colwid->clearFocus();
    }
}

#include <iostream.h>
/**
 * Overloaded QListViewItem::paintCell() method. This method tests
 * if we  have a widget  prepared for this  cell and if we have it
 * shows and moves or hides it.
*/		
void AdvListViewItem::paintCell(QPainter *p, const QColorGroup &cg,
                                int column, int width, int align)
{
    if ( !p )
        return;

    QListView *lv = listView();
    
    // Widget activated?
    if (colwid && column == 1) {
            
#if defined(LVDEBUG)
        debug("paintCell %s, (selected=%d)", text(column), isSelected());
#endif
        
        // if the line the class  builds is selected and if the
        //   widget for this column is active we can move it to
        //   the right place  and show it  otherwise we have to
        //   hide and take the focus away from it.
        
        if (isSelected()) {
            // This all should not be done in paintEvent(), but in a function
            // that is called on select and resize.
            QRect r(lv->itemRect( this ));
            
            // test if the column is _completely_ visible by testing
            //   wheter it is  not starting at y 0 or,  if it is the
            //   first item, whether it is not starting at y under 0
            if ( (r.y()>0) || ((lv->firstChild() == this) && (r.y()>=0)) ) {
                int x = lv->columnWidth(0);
                int y = r.y() + lv->viewport()->y();
                //int x = (int)p->worldMatrix().dx();
                
                // we have to test if the colums goes out of the
                //   visible (x or width) area and if it does we
                //   have to resize it in order to avoid that it
                //   overpaintes the vertical scrollbar.
                int vs = 0;
                if (lv->verticalScrollBar()->isVisible())
                    vs = lv->verticalScrollBar()->width();
                int w = width;
                if (x+width > lv->width() - vs)
                    w = lv->width() - vs - x;
                int h = height();
                
                colwid->setGeometry(x, y, w, h);
                colwid->show();
                colwid->setFocus();
            } else {
                colwid->hide();
#if 0
                if (colwid->getDisplayedText().isEmpty())
                    setText(1, colwid->getText());
                else
                    setText(1, colwid->getDisplayedText());
#endif
            }
            
        } else {
            // since this  widget has been  disactivated or the line this
            //   instance of the class represents is not selected we have
            //   to hide the widget and take the focus away from it
            colwid->hide();
            colwid->clearFocus();
        }
    }
    
    QListViewItem::paintCell(p, cg, column, width, align);
}
