/***************************************************************************
 *   Copyright 2003-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *   Copyright 2007 Ralf Habacker  <Ralf.Habacker@freenet.de>       *
 *   Copyright 2006-2007 Matt Rogers  <mattr@kde.org>               *
 *   Copyright 2006-2007 Hamish Rodda <rodda@kde.org>               *
 *   Copyright 2005-2007 Adam Treat <treat@kde.org>                 *
 *   Copyright 2003-2007 Jens Dagerbo <jens.dagerbo@swipnet.se>     *
 *   Copyright 2001-2002 Bernd Gehrmann <bernd@mail.berlios.de>     *
 *   Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>*
 *   Copyright 2003 Roberto Raggi <roberto@kdevelop.org>            *
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

#include <config.h>

#include <QPixmap>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>

int main( int argc, char *argv[] )
{
    KAboutData aboutData( "kdevelop", 0, ki18n( "KDevelop" ),
                          VERSION, ki18n("blah"), KAboutData::License_GPL,
                          ki18n( "(c) 1999-2007, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org" );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication app;

    for (int i = 0; i < 260; ++i)
    KIconLoader::global()->loadIcon("CVprotected_var", KIconLoader::Small);
}

