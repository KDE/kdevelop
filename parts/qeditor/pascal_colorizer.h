/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PASCAL_COLORIZER_H
#define PASCAL_COLORIZER_H

#include <qfont.h>

#include "qsourcecolorizer.h"

class PascalColorizer: public QSourceColorizer
{
public:
    PascalColorizer( QEditor* );
    virtual ~PascalColorizer();

};

#endif
