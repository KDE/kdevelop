/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __cppsupport_utils_h
#define __cppsupport_utils_h

#include <qmap.h>
#include <qstringlist.h>

class CodeModel;

QStringList typeNameList( const CodeModel* model );
QMap<QString, QString> typedefMap( const CodeModel* model );

#endif // __cppsupport_utils_h 
// kate: indent-mode csands; tab-width 4;

