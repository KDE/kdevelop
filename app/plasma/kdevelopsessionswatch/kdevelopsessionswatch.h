/*  This file is part of KDevelop
    Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOPSESSIONSWATCH_H
#define KDEVELOPSESSIONSWATCH_H

// library
#include <kdevelopsessionswatch_export.h>

class QString;
class QObject;

namespace KDevelopSessionsWatch {

// not exactly matching the scope of the watch, but let's put this util method here for now
KDEVELOPSESSIONSWATCH_EXPORT
void openSession(const QString& sessionId);

KDEVELOPSESSIONSWATCH_EXPORT
void registerObserver(QObject* observer);

KDEVELOPSESSIONSWATCH_EXPORT
void unregisterObserver(QObject* observer);

}

#endif
