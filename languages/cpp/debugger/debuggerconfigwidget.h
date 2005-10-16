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

#include "debuggerconfigwidgetbase.h"

#include <qdom.h>

namespace GDBDebugger
{

class DebuggerPart;

class DebuggerConfigWidget : public DebuggerConfigWidgetBase
{
    Q_OBJECT

public:
    DebuggerConfigWidget( DebuggerPart* part, QWidget *parent=0, const char *name=0 );
    ~DebuggerConfigWidget();

public slots:
    void accept();

private:
    QDomDocument &dom;
};

}

#endif
