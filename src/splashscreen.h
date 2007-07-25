/***************************************************************************
 *   Copyright 2003-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *   Copyright 2003-2007 Jens Dagerbo <jens.dagerbo@swipnet.se>     *
 *   Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>*
 *   Copyright 2003 Caleb Tennis <caleb@gentoo.org>                 *
 *   Copyright 2003 George Staikos <staikos@kde.org>                *
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
#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include <ksplashscreen.h>
class QPixmap;
class QPainter;

class SplashScreen : public KSplashScreen
{
    Q_OBJECT

public:
    SplashScreen( const QPixmap& pixmap = QPixmap(), Qt::WindowFlags f = 0 ); //krazy:exclude-explicit
    virtual ~SplashScreen();

protected:
    void drawContents( QPainter * painter );

public slots:
    void animate();
    void showMessage( const QString &str, int flags = Qt::AlignLeft,
                      const QColor &color = Qt::black );

private:
    int m_state;
    int m_progress_bar_size;
    QString m_string;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
