/***************************************************************************
                             grepviewconfigwidget.h
                             ----------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef GREPVIEWCONFIGWIDGET_H
#define GREPVIEWCONFIGWIDGET_H

#include "customizedlg.h"

class GrepViewConfigWidget : public QWidget
{
    Q_OBJECT
	
public: 
    GrepViewConfigWidget( QWidget *parent, const char *name=0 );
    ~GrepViewConfigWidget();
};

#endif
