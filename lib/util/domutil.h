/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOMUTIL_H_
#define _DOMUTIL_H_

#include <qdom.h>


class DomUtil
{
public:
    static QString readEntry(const QDomDocument &doc, const QString &path);
    static int readIntEntry(const QDomDocument &doc, const QString &path);
    static bool readBoolEntry(const QDomDocument &doc, const QString &path);
    static void writeEntry(QDomDocument &doc, const QString &path, const QString &value);
    static void writeIntEntry(QDomDocument &doc, const QString &path, int value);
    static void writeBoolEntry(QDomDocument &doc, const QString &path, bool value);
};

#endif
    
