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

#ifndef _CLASSVIEWCONFIGWIDGET_H_
#define _CLASSVIEWCONFIGWIDGET_H_

#include "classviewconfigwidgetbase.h"

class ClassViewPart;


class ClassViewConfigWidget : public ClassViewConfigWidgetBase
{
    Q_OBJECT

public:
    ClassViewConfigWidget( ClassViewPart *widget, QWidget *parent, const char *name=0 );
    ~ClassViewConfigWidget();

public slots:
     void accept();

private:
    void readConfig();
    void storeConfig();
    
    ClassViewPart *m_part;
};

#endif
