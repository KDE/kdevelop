/* This file is part of the KDE project
   Copyright (C) 2002 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KMDI_DOCK_CONTAINER_
#define _KMDI_DOCK_CONTAINER_

#include <qwidget.h>
#include <qstringlist.h>
#include <kdockwidget.h>
#include <qmap.h>
#include <qdom.h>

# include <kdockwidget_p.h>

#include <qpushbutton.h>

class QWidgetStack;
class KMultiTabBar;
class KDockButton_Private;

class KMdiDockContainer: public QWidget, public KDockContainer
{
  Q_OBJECT

  public:
    KMdiDockContainer(QWidget *parent, QWidget *win, int position, int flags);
    virtual ~KMdiDockContainer();

    KDockWidget *parentDockWidget();

    virtual void insertWidget (KDockWidget *w, QPixmap, const QString &, int &);
    virtual void showWidget (KDockWidget *w);
    virtual void setToolTip (KDockWidget *, QString &);
    virtual void setPixmap(KDockWidget*,const QPixmap&);
    virtual void undockWidget(KDockWidget*);
    virtual void removeWidget(KDockWidget*);

    void hideIfNeeded();

    virtual void save(KConfig *,const QString& group_or_prefix);
    virtual void load(KConfig *,const QString& group_or_prefix);

    virtual void save(QDomElement&);
    virtual void load(QDomElement&);

    void setStyle(int);

  public slots:
    void init();
    void collapseOverlapped();
    void toggle();
    void nextToolView();
    void prevToolView();
  protected slots:
    void tabClicked(int);
    void delayedRaise();
    void changeOverlapMode();
  private:
    QWidget *m_mainWin;
    QWidgetStack *m_ws;
    KMultiTabBar *m_tb;
    int mTabCnt;
    int oldtab;
    int m_previousTab;
    int m_position;
    int m_separatorPos;
    QMap<KDockWidget*,int> m_map;
    QMap<int,KDockWidget*> m_revMap;
    QMap<KDockWidget*,KDockButton_Private*> m_overlapButtons;
    QStringList itemNames;
    QMap<QString,QString> tabCaptions;
    QMap<QString,QString> tabTooltips;
    int m_inserted;
    int m_delayedRaise;
    bool m_vertical;
    bool m_block;
    bool m_tabSwitching;

  signals:
        void activated(KMdiDockContainer*);
        void deactivated(KMdiDockContainer*);
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
