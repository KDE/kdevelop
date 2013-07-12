/***************************************************************************
 *   Copyright 2013 Sven Brauch <svenbrauch@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVSPLASH_H
#define KDEVSPLASH_H

#include <ksplashscreen.h>
#include <QPainter>
#include <QApplication>
#include <qgraphicsitem.h>
#include <QDeclarativeView>
#include <QDebug>
#include <KStandardDirs>

// This class is responsible for drawing the KDevelop splashscreen since
// version 4.6. A background image is passed in from outside, which
// contains the KDevelop logo and the text "KDevelop" and the version number
// on a solid background.
// This class draws a partially transparent QML scene on top of that image.
class KDevSplashScreen : public KSplashScreen {
Q_OBJECT
public:
    explicit KDevSplashScreen(const QPixmap& pixmap, Qt::WindowFlags f = 0);
    virtual ~KDevSplashScreen();
    virtual void drawContents(QPainter* painter);

public slots:
    void progress(int progress);

private:
    QDeclarativeView* m_view;
    int m_progress;
};

#endif
