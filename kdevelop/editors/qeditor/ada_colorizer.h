/* Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef ADA_COLORIZER_H
#define ADA_COLORIZER_H

#include "qsourcecolorizer.h"

// class QTextParagraph;

class AdaColorizer: public QSourceColorizer
{
public:
    AdaColorizer (QEditor *);
    virtual ~AdaColorizer ();

    int computeLevel (QTextParagraph* parag, int startLevel);
};

#endif
