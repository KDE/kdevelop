/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "envvartools.h"

#include <qregexp.h>

QString EnvVarTools::quote( const QString & arg )
{
    QString res = arg;
    res.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("'\\''"));
    res.prepend("\"");
    res.append("\"");
    return res;
}

