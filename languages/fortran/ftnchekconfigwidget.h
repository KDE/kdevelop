/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FTNCHEKCONFIGWIDGET_H_
#define _FTNCHEKCONFIGWIDGET_H_

#include <qdom.h>
#include "ftnchekconfigwidgetbase.h"

class Q3ButtonGroup;

class FtnchekConfigWidget : public FtnchekConfigWidgetBase
{
    Q_OBJECT
    
public:
    FtnchekConfigWidget(QDomDocument &projectDom, QWidget *parent, const char *name);
    ~FtnchekConfigWidget();

public slots:
    void accept();
    
private:
    void readConfig();
    void storeConfig();

    Q3ButtonGroup *arguments_group, *common_group;
    Q3ButtonGroup *truncation_group, *usage_group;
    Q3ButtonGroup *f77_group, *portability_group;
    QDomDocument dom;
};

#endif
