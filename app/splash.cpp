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
    setFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    setResizeMode(QQuickView::SizeViewToRootObject);

    setSource(QUrl(QStringLiteral("qrc:/kdevelop/splash.qml")));
    if (!rootObject()) {
        qWarning() << "Could not load KDevelop splash screen";
        hide(); // hide instead of showing garbage
        return;
    }

    if (rootObject()) {
        rootObject()->setProperty("appIcon",
            QUrl::fromLocalFile(KIconLoader().iconPath("kdevelop", -48)));
        rootObject()->setProperty("appVersionMajor", VERSION_MAJOR);
        rootObject()->setProperty("appVersionMinor", VERSION_MINOR);
        rootObject()->setProperty("appVersionPatch", VERSION_PATCH);
    }

    QRect geo = geometry();
    geo.moveCenter(screen()->geometry().center());
    setGeometry(geo);
}

KDevSplashScreen::~KDevSplashScreen()
{
}

void KDevSplashScreen::progress(int progress)
{
    // notify the QML script of the progress change
    if (rootObject()) {
        rootObject()->setProperty("progress", progress);
    }

    // note: We don't have an eventloop running, hence we need to call both processEvents and sendPostedEvents here
    // DeferredDelete events alone won't be processed until sendPostedEvents is called
    // also see: http://osxr.org/qt/source/qtbase/tests/auto/widgets/kernel/qapplication/tst_qapplication.cpp#1401
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->sendPostedEvents(0, QEvent::DeferredDelete);
}
