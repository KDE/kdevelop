/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DETAILSLISTVIEW_H
#define DETAILSLISTVIEW_H

#include <klistview.h>

class QHBox;
class GenericProjectPart;

class DetailsListView : public KListView
{
    Q_OBJECT
public:
    DetailsListView(GenericProjectPart* part, QWidget* parent, const char *name = 0);
    virtual ~DetailsListView();
};

#endif
