
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
#include <kmdidockwidget.h>
#include <qmap.h>

#include <qpushbutton.h>

class QWidgetStack;
class KMultiTabBar;
class KDockButton_Private;

class KMdiDockContainer: public QWidget, public KMdiDockContainerBase
{
  Q_OBJECT

  public:
    KMdiDockContainer(QWidget *parent, QWidget *win, int position);
    virtual ~KMdiDockContainer();
    KDockWidget *parentDockWidget();

    virtual void insertWidget (KDockWidget *w, QPixmap, const QString &, int &);
    virtual void showWidget (KDockWidget *w);
    virtual void setToolTip (KDockWidget *, QString &);
    virtual void setPixmap(KDockWidget*,const QPixmap&);
    virtual void undockWidget(KDockWidget*);
    virtual void removeWidget(KDockWidget*);

    void hideIfNeeded();
#ifndef NO_KDE2
    virtual void save(KConfig *,const QString& group_or_prefix);
    virtual void load(KConfig *,const QString& group_or_prefix);
#endif

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
	QMap<KMdiDockWidget*,int> m_map;
	QMap<int,KMdiDockWidget*> m_revMap;
	QMap<KMdiDockWidget*,KDockButton_Private*> m_overlapButtons;
    QStringList itemNames;
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
