/* This file is part of the KDE project
   Copyright (C) 2001 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _SPLITTER_H_
#define _SPLITTER_H_

#include <qvaluelist.h>
#include <qframe.h>
#include <qptrlist.h>

class SplitterHandle;


/**
 * A special splitter that lacks many of the features of QSplitter,
 * but has some other features we need: One can replace one widget
 * by another, add widgets at an arbitrary position in the chain,
 * and the user can resize the widgets by moving a SplitterBar
 * (which is much easier to grab than a QSplitterHandle) around.
 * Because of these features, widgets are not automatically embedded
 * by using the splitter as parent.
 */
class Splitter : public QWidget
{
    Q_OBJECT
    
public:
    Splitter( QWidget *parent=0, const char *name=0 );
    ~Splitter();

    void setOrientation(Orientation orient);
    Orientation orientation() const;

    /**
     * Returns true if there is more than one child.
     */
    bool hasMultipleChildren() const;
    /**
     * Returns the first child. This is useful for
     * destroying the splitter when the number of
     * children has gone down to one. See TopLevel::eventFilter().
     */
    QWidget *firstChild();
    
    /**
     * Adds the widget w to the list of managed children.
     */
    void addChild(QWidget *w);
    /**
     * Splits the space covered by old into two halves and embeds
     * the widget w into the second. Note: w must have the splitter
     * as parent.
     */
    void splitChild(QWidget *old, QWidget *w);
    /**
     * Puts the widget w into the space formerly covered by
     * old. old is not destroyed. Note: w must have the splitter
     * as parent.
     */
    void replaceChild(QWidget *old, QWidget *w);
    /**
     * Moves the splitter after widget w to the vertical
     * offset pos.
     */
    void moveSplitter(QWidget *w, int pos);

signals:
    void collapsed(Splitter *splitter);

protected:
    virtual QSize minimumSizeHint() const;
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private slots:
    void collapse();
    
private:
    void doLayout();
    int handleWidth() const;
    
    QPtrList<QWidget> childlist;
    QPtrList<SplitterHandle> handlelist;
    QValueList<int> sizes; // should really be a QVector
    Orientation _or;
};


/**
 * If a SplitterBar is a child of a widget which in turn is
 * embedded in a Splitter, it can be used for resizing the
 * embedded widget.
 */
class SplitterBar : public QFrame
{
    Q_OBJECT

public:
    SplitterBar( QWidget *parent=0, const char *name=0 );
    ~SplitterBar();

protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void childEvent(QChildEvent *e);
    virtual bool eventFilter(QObject *o, QEvent *e);

private:
    int offset;
};


/**
 * The handle which separates children in a horizontal Splitter.
 * In contrast to SplitterBar, this is only used internally.
 */
class SplitterHandle : public QWidget
{
public:
    SplitterHandle( QWidget *embeddedSibling, Splitter *parent, const char *name=0 );
    ~SplitterHandle();

protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *);

private:
    int offset;
    QWidget *sibling;
};

#endif
