/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Hamish Rodda                                    *
 *   rodda@kde.org                                                         *
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

#include <kcmodule.h>

namespace GDBDebugger
{

class DebuggerConfigWidget : public KCModule, public Ui::DebuggerConfigWidget
{
    Q_OBJECT

public:
    DebuggerConfigWidget( QWidget *parent, const QVariantList &args );
    ~DebuggerConfigWidget();
};

}

#endif
