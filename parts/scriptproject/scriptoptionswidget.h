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

#ifndef _SCRIPTPROJECTOPTIONSWIDGET_H_
#define _SCRIPTPROJECTOPTIONSWIDGET_H_

#include "scriptoptionswidgetbase.h"

class KDevPlugin;


class ScriptOptionsWidget : public ScriptOptionsWidgetBase
{
    Q_OBJECT
    
public:
    ScriptOptionsWidget( KDevPlugin *part, QWidget *parent=0, const char *name=0 );
    ~ScriptOptionsWidget();

public slots:
    void accept();

private:
    KDevPlugin *m_part;
};

#endif
