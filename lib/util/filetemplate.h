/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILETEMPLATE_H_
#define _FILETEMPLATE_H_

#include <qstring.h>

class KDevPart;


class FileTemplate
{
public:
    static QString read(KDevPart *part, const QString &relName);
};

#endif
