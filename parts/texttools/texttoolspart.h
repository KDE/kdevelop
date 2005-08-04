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

#ifndef _TEXTTOOLSPART_H_
#define _TEXTTOOLSPART_H_

#include <qpointer.h>
#include <kparts/part.h>
#include "kdevplugin.h"

class TextToolsWidget;


class TextToolsPart : public KDevPlugin
{
    Q_OBJECT

public:
    TextToolsPart( QObject *parent, const char *name, const QStringList & );
    ~TextToolsPart();

private slots:
    void activePartChanged(KParts::Part *part);

private:
    void createWidget();
    
    QPointer<TextToolsWidget> m_widget;
};

#endif
