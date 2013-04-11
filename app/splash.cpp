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

#include "splash.h"

KDevSplashScreen::KDevSplashScreen(const QPixmap& pixmap, Qt::WindowFlags f)
    : KSplashScreen(pixmap, f | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
    , m_view(new QDeclarativeView)
{
    QString splashScript = KStandardDirs::locate("data", "kdevelop/splash.qml");
    setFixedSize(pixmap.size());
    m_view->setSource(QUrl(splashScript));
    if ( ! m_view->rootObject() ) {
        return;
    }
    m_view->resize(pixmap.size());
}

KDevSplashScreen::~KDevSplashScreen()
{
    delete m_view;
}

void KDevSplashScreen::drawContents(QPainter* painter)
{
    m_view->render(painter);
}

void KDevSplashScreen::progress(int progress)
{
    if ( ! m_view->rootObject() ) {
        return;
    }
    m_progress = progress;
    // notify the QML script of the progress change
    m_view->rootObject()->setProperty("progress", m_progress);
    repaint();
}
