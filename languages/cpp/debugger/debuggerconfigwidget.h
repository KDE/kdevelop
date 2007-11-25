/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DEBUGGERCONFIGWIDGET_H_
#define _DEBUGGERCONFIGWIDGET_H_

#include "ui_debuggerconfigwidget.h"

#include <qdom.h>

namespace GDBDebugger
{

class CppDebuggerPlugin;

class DebuggerConfigWidget : public QWidget, public Ui::DebuggerConfigWidget
{
    Q_OBJECT

public:
    DebuggerConfigWidget( CppDebuggerPlugin* plugin, QWidget *parent=0 );
    ~DebuggerConfigWidget();

public Q_SLOTS:
    void accept();

private:
    CppDebuggerPlugin* m_plugin;
};

}

#endif
