/***************************************************************************
                          kmultitabbar_p.h -  description
                             -------------------
    begin                :  2003
    copyright            : (C) 2003 by Joseph Wenninger <jowenn@kde.org>
 ***************************************************************************/

/***************************************************************************
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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef K_MULTI_TAB_BAR_P_H
#define K_MULTI_TAB_BAR_P_H
#include <qscrollview.h>
#include <kmultitabbar.h>

class KMultiTabBarInternal: public QScrollView
{
        Q_OBJECT
public:
        KMultiTabBarInternal(QWidget *parent,KMultiTabBar::KMultiTabBarMode bm);
        int appendTab(const QPixmap &,int=-1,const QString& =QString::null);
        KMultiTabBarTab *tab(int) const;
        void removeTab(int);
        void setPosition(enum KMultiTabBar::KMultiTabBarPosition pos);
        void setStyle(enum KMultiTabBar::KMultiTabBarStyle style);
        void showActiveTabTexts(bool show);
        QPtrList<KMultiTabBarTab>* tabs(){return &m_tabs;}
private:
        friend class KMultiTabBar;
        QWidget *box;
	QBoxLayout *mainLayout;
        QPtrList<KMultiTabBarTab> m_tabs;
        enum KMultiTabBar::KMultiTabBarPosition m_position;
        bool m_showActiveTabTexts;
        enum  KMultiTabBar::KMultiTabBarStyle m_style;
	int m_expandedTabSize;
	int m_lines;
	KMultiTabBar::KMultiTabBarMode m_barMode;
protected:
	virtual bool eventFilter(QObject *,QEvent*);
        virtual void drawContents ( QPainter *, int, int, int, int);

        /**
         * [contentsM|m]ousePressEvent are reimplemented from QScrollView
         * in order to ignore all mouseEvents on the viewport, so that the
         * parent can handle them.
         */
        virtual void contentsMousePressEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
};
#endif

