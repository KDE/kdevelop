/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#ifndef KTABWIDGET_H
#define KTABWIDGET_H

#include <qtabwidget.h>

class KTabWidgetPrivate;

/**
 * @since 3.2
 */
class KTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY( bool tabReorderingEnabled READ isTabReorderingEnabled WRITE setTabReorderingEnabled )
    Q_PROPERTY( bool hoverCloseButton READ hoverCloseButton WRITE setHoverCloseButton )
    Q_PROPERTY( bool hoverCloseButtonDelayed READ hoverCloseButtonDelayed WRITE setHoverCloseButtonDelayed )

public:
    KTabWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    /**
     * Destructor.
     */
    virtual ~KTabWidget();
    /*!
      Set the tab of the given widget to \a color.
    */
    void setTabColor( QWidget *, const QColor& color );
    QColor tabColor( QWidget * ) const;

    /*!
      Returns true if tab ordering with the middle mouse button
      has been enabled.
    */
    bool isTabReorderingEnabled() const;

    /*!
      Returns true if the close button is shown on tabs
      when mouse is hovering over them.
    */
    bool hoverCloseButton() const;

    /*!
      Returns true if the close button is shown on tabs
      after a delay.
    */
    bool hoverCloseButtonDelayed() const;

public slots:
    /*!
      Move a widget's tab from first to second specified index and emit
      signal movedTab( int, int ) afterwards.
    */
    virtual void moveTab( int, int );

    /*!
      If \a enable is true, tab reordering with middle button will be enabled.

      Note that once enabled you shouldn't rely on previously queried
      currentPageIndex() or indexOf( QWidget * ) values anymore.

      You can connect to signal movedTab(int, int) which will notify
      you from which index to which index a tab has been moved.
    */
    void setTabReorderingEnabled( bool enable );

    /*!
      If \a enable is true, a close button will be shown on mouse hover
      over tab icons which will emit signal closeRequest( QWidget * )
      when pressed.
    */
    void setHoverCloseButton( bool enable );

    /*!
      If \a delayed is true, a close button will be shown on mouse hover
      over tab icons after mouse double click delay else immediately.
    */
    void setHoverCloseButtonDelayed( bool delayed );



signals:
    /*!
      Connect to this and set accept to true if you can and want to decode the event.
    */
    void testCanDecode(const QDragMoveEvent *e, bool &accept /* result */);

    /*!
      Received an event in the empty space beside tabbar. Usually creates a new tab.
      This signal is only possible after testCanDecode and positive accept result.
    */
    void receivedDropEvent( QDropEvent * );

    /*!
      Received an drop event on given widget's tab.
      This signal is only possible after testCanDecode and positive accept result.
    */
    void receivedDropEvent( QWidget *, QDropEvent * );

    /*!
      Request to start a drag operation on the given tab.
    */
    void initiateDrag( QWidget * );

    /*!
      The right mouse button was pressed over empty space besides tabbar.
    */
    void contextMenu( const QPoint & );

    /*!
      The right mouse button was pressed over a widget.
    */
    void contextMenu( QWidget *, const QPoint & );

    /*!
      A tab was moved from first to second index. This signal is only
      possible after you have called setTabReorderingEnabled( true ).
    */
    void movedTab( int, int );

    /*!
      A double left mouse button click was performed over the widget.
    */
    void mouseDoubleClick( QWidget * );

    /*!
      A middle mouse button click was performed over empty space besides tabbar.
    */
    void mouseMiddleClick();

    /*!
      A middle mouse button click was performed over the widget.
    */
    void mouseMiddleClick( QWidget * );

    /*!
      The close button of a widget's tab was clicked. This signal is
      only possible after you have called setHoverCloseButton( true ).
    */
    void closeRequest( QWidget * );

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dropEvent( QDropEvent * );

protected slots:
    virtual void receivedDropEvent( int, QDropEvent * );
    virtual void initiateDrag( int );
    virtual void contextMenu( int, const QPoint & );
    virtual void mouseDoubleClick( int );
    virtual void mouseMiddleClick( int );
    virtual void closeRequest( int );

private:
    bool isEmptyTabbarSpace( const QPoint & )  const;

    KTabWidgetPrivate *d;
};

#endif
