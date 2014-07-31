/***************************************************************************
 *   Copyright 2013 Sven Brauch <svenbrauch@gmail.com>                     *
 *   Copyright 2013 Milian Wolff <mail@milianw.de>                         *
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

#include <KDebug>
#include <KIconLoader>

#include <KStandardDirs>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <qscreen.h>

#include "config.h"

KDevSplashScreen::KDevSplashScreen()
    : QQuickView()
{
    setFlags(Qt::FramelessWindowHint | Qt::Tool);

    QRect geo(0,0, 475, 301);
    geo.moveCenter(screen()->geometry().center());
    setMaximumSize(geo.size());
    setMinimumSize(geo.size());
    setGeometry(geo);

    engine()->rootContext()->setContextProperty("appIcon", KIconLoader().iconPath("kdevelop", -48));
    engine()->rootContext()->setContextProperty("appVersionMajor", VERSION_MAJOR);
    engine()->rootContext()->setContextProperty("appVersionMinor", VERSION_MINOR);
    engine()->rootContext()->setContextProperty("appVersionPatch", VERSION_PATCH);

    QString splashScript = KStandardDirs::locate("data", "kdevelop/splash.qml");
    setSource(QUrl::fromLocalFile(splashScript));
    if ( !rootObject() ) {
        kWarning() << "Could not find KDevelop splash screen: kdevelop/splash.qml" << splashScript;
    }
}

KDevSplashScreen::~KDevSplashScreen()
{
}

void KDevSplashScreen::progress(int progress)
{
    Q_ASSERT(rootObject());

    // notify the QML script of the progress change
    rootObject()->setProperty("progress", progress);
    qApp->processEvents();
}
