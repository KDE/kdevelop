/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPCONFIGWIDGET_H_
#define _GREPCONFIGWIDGET_H_


#include <qwidget.h>


class GrepConfigWidget : public QWidget
{
    Q_OBJECT
	
public: 
    GrepConfigWidget( QWidget *parent, const char *name=0 );
    ~GrepConfigWidget();
};

#endif
