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

#ifndef KDEVSPLASH_H
#define KDEVSPLASH_H

#include <ksplashscreen.h>

class QDeclarativeView;

/**
 * This class is draws the KDevelop splashscreen.
 *
 * The actualy splash design can be adapted via splash.qml
 */
class KDevSplashScreen : public KSplashScreen
{
    Q_OBJECT
public:
    explicit KDevSplashScreen();
    virtual ~KDevSplashScreen();
    virtual void drawContents(QPainter* painter);

public slots:
    void progress(int progress);

private:
    QScopedPointer<QDeclarativeView> m_view;
};

#endif
