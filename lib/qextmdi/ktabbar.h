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

#ifndef KTABBAR_H
#define KTABBAR_H

#include <qtabbar.h>

class QTimer;
class QPushButton;
class KTabBarPrivate;

/**
 * @since 3.2
 */
class KTabBar: public QTabBar
{
    Q_OBJECT

public:
    KTabBar( QWidget* parent=0, const char* name=0 );
    virtual ~KTabBar();

    virtual void setTabEnabled( int, bool );

    const QColor &tabColor( int ) const;
    void setTabColor( int, const QColor& );
    virtual void removeTab( QTab * );

    void setTabReorderingEnabled( bool enable );
    bool isTabReorderingEnabled() const;

    void setHoverCloseButton( bool );
    bool hoverCloseButton() const;

    void setHoverCloseButtonDelayed( bool );
    bool hoverCloseButtonDelayed() const;

signals:
    void contextMenu( int, const QPoint & );
    void mouseDoubleClick( int );
    void mouseMiddleClick( int );
    void initiateDrag( int );
    void testCanDecode(const QDragMoveEvent *e, bool &accept /* result */);
    void receivedDropEvent( int, QDropEvent * );
    void moveTab( int, int );
    void closeRequest( int );

protected:
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );

    virtual void dragMoveEvent( QDragMoveEvent *e );
    virtual void dropEvent( QDropEvent *e );

    virtual void paintLabel( QPainter*, const QRect&, QTab*, bool ) const;

protected slots:
    virtual void closeButtonClicked();
    virtual void onLayoutChange();
    virtual void enableCloseButton();
    virtual void activateDragSwitchTab();

private:
    QPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    QMap<int, QColor> mTabColors;
    QTab *mHoverCloseButtonTab, *mDragSwitchTab;
    QPushButton *mHoverCloseButton;
    QTimer *mEnableCloseButtonTimer, *mActivateDragSwitchTabTimer;

    bool mHoverCloseButtonEnabled;
    bool mHoverCloseButtonDelayed;
    bool mTabReorderingEnabled;

    KTabBarPrivate * d;
};

#endif
