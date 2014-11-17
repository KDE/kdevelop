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

#include <KIconLoader>


#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <qscreen.h>
#include <QStandardPaths>
#include <QDebug>

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

    engine()->rootContext()->setContextProperty("appIcon",
        QUrl::fromLocalFile(KIconLoader().iconPath("kdevelop", -48)));
    engine()->rootContext()->setContextProperty("appVersionMajor", VERSION_MAJOR);
    engine()->rootContext()->setContextProperty("appVersionMinor", VERSION_MINOR);
    engine()->rootContext()->setContextProperty("appVersionPatch", VERSION_PATCH);

    QString splashScript = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdevelop/splash.qml");
    setSource(QUrl::fromLocalFile(splashScript));
    if ( !rootObject() ) {
        qWarning() << "Could not find KDevelop splash screen: kdevelop/splash.qml" << splashScript;
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

    // note: We don't have an eventloop running, hence we need to call both processEvents and sendPostedEvents here
    // DeferredDelete events alone won't be processed until sendPostedEvents is called
    // also see: http://osxr.org/qt/source/qtbase/tests/auto/widgets/kernel/qapplication/tst_qapplication.cpp#1401
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->sendPostedEvents(0, QEvent::DeferredDelete);
}
