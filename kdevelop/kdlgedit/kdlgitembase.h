/***************************************************************************
                           kdlgitembase.h  -  
                             -------------------
    begin                : Thu Apr 08 1999
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


#ifndef _KDLGITEMBASE_H_
#define _KDLGITEMBASE_H_

#include <qwidget.h>


class KDlgEditWidget;
class KDlgPropertyBase;
class KDlgItemDatabase;
class KDlgItem_QWidget;

/**
  * @short Base item inherited by each complexer item.
  *
  * This is the base item inherited by each other item. It defines
  * the default methods and variables needed for each and every item.
  */
class KDlgItem_Base : public QObject
{
  Q_OBJECT

public:
    enum Role { Main, Container, Widget };

    KDlgItem_Base(KDlgEditWidget *editWidget, KDlgItem_Base *parentItem, Role role);
    virtual ~KDlgItem_Base() { }

    /**
     * The role the item is playing.
     * It can only be set in the constructor.
     */
    Role role()
        { return rol; }
    /**
     * The 'real' widget that this object is representing.
     * It can only be set in the constructor.
     */
    QWidget *widget()
        { return wid; }
    /**
     * The KDlgEditWidget object which created this item.
     */
    KDlgEditWidget* getEditWidget()
        { return editwid; }
    /**
     * The parent item of this item. This is always a container.
     */
    KDlgItem_Base *parentItem()
        { return parentit; }
    /**
     * Whether this item is selected.
     */
    bool isSelected()
        { return isItemActive; }
    
    virtual void recreateItem();

    /**
     * Returns the type of the items class. Has to be overloaded in order to return the right type.
     * (i.e. returns "QPushButton" for a PushButton item.
    */
    virtual const char *itemClass()
        { return widget()->className(); }

    /**
     * Rebuilds the item from its properties. If <i>it</i> is 0 the
     * idget stored in this class (<i>item</i>), otherwise
     * the one <i>it</i> points to is changed.
     * If you reimplement this method (and you should do so) you
     * can call repaintItem(item) in order to let your item be handled.
     * This makes sense since every widget in QT inherites QWidget so
     * you won't need to set the properties of the QWidget in your code.
    */
    virtual void repaintItem();

    /**
     * Returns a pointer to the properties of this item. See KDlgPropertyBase for
     * more details.
    */
    KDlgPropertyBase *getProps() { return props; }

    /**
     * Returns a pointer to the child database. If you call this method for
     * a KDlgItem_QWidget you´ll get a pointer otherwise 0 because only a QWidget
     * may contain childs.
    */
    KDlgItemDatabase *getChildDb() { return childs; }

    /**
     * Returns the number child items if called for a KDlgItem_Widget otherwise 0.
    */
    int getNrOfChilds();

    /**
     * Adds a child item to the children database.(Use only for KDlgItem_Widget's!!)<br><br>
     * Returns true if successful, otherwise false.
    */
    bool addChild(KDlgItem_Base *itm);

    /**
     * Has to be overloaded ! Sets the state if this item to selected. (That means the border and the rectangles are painted)
    */
    virtual void select();

    /**
     * Has to be overloaded ! Sets the state if this item to not selected. (That means the border and the rectangles are NOT painted)
    */
    virtual void deselect();

    /**
     * Removes this item including all children (if a KDlgItem_Widget) from the dialog.
    */
    void deleteMyself();

    void execContextMenu();
    
protected:
    void setWidget(QWidget *widget);

    virtual bool eventFilter( QObject *o, QEvent *e);
    int Prop2Bool(QString name);
    int Prop2Int(QString name, int defaultval=0);
    QString Prop2Str(QString name);

    KDlgItemDatabase *childs;
    KDlgPropertyBase *props;

private:
    enum Corner { NoCorner, TopLeft, TopRight, BottomLeft, BottomRight,
                  MiddleTop, MiddleBottom, MiddleLeft, MiddleRight };
                  
    Corner cornerForPos(QPoint pos);
    bool getResizeCoords(Corner c, int diffx, int diffy, int *x, int *y, int *w, int *h);
    void setMouseCursorToEdge(Corner c);
    void moveRulers(QPoint relpos);
    void paintCorners(QPainter *p);
    void paintGrid(QPainter *p);
    void widgetPaintEvent(QWidget *w, QPaintEvent *e);
    void widgetMousePressEvent(QWidget *w, QMouseEvent *e);
    void widgetMouseReleaseEvent(QWidget *w, QMouseEvent *e);
    void widgetMouseMoveEvent(QWidget *w, QMouseEvent *e);
    
    Role rol;
    QWidget *wid;
    KDlgEditWidget* editwid;
    KDlgItem_Base *parentit;
    bool isItemActive;
    bool inPaintEvent;
    Corner pressedEdge;
    QPoint startPnt, lastPnt;
    QRect origRect;
    bool isMBPressed;
};



#endif


