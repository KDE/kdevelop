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
#ifndef OVERVIEWLISTVIEW_H
#define OVERVIEWLISTVIEW_H

#include <klistview.h>

class QHBox;
class GenericProjectPart;

class OverviewListView : public KListView
{
    Q_OBJECT
public:
    OverviewListView( GenericProjectPart* part, QWidget *parent, const char *name = 0 );
    virtual ~OverviewListView();
};

#endif
