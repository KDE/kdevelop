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

#ifndef _CONFIGUREOPTIONSWIDGET_H_
#define _CONFIGUREOPTIONSWIDGET_H_

#include "configureoptionswidgetbase.h"

class AutoProjectPart;


class ConfigureOptionsWidget : public ConfigureOptionsWidgetBase
{
    Q_OBJECT
    
public:
    ConfigureOptionsWidget( AutoProjectPart *part, QWidget *parent=0, const char *name=0 );
    ~ConfigureOptionsWidget();

public slots:
    void accept();

private:

    AutoProjectPart *m_part;
};

#endif
