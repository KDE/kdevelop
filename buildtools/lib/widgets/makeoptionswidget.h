/* This file is part of the KDE project
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _MAKEOPTIONSWIDGET_H_
#define _MAKEOPTIONSWIDGET_H_

#include "makeoptionswidgetbase.h"

#include <qdom.h>

class EnvironmentVariablesWidget;

/**
Make options widget.
*/
class MakeOptionsWidget : public MakeOptionsWidgetBase
{
    Q_OBJECT
    
public:
    MakeOptionsWidget( QDomDocument &dom, const QString &configGroup,
                       QWidget *parent=0, const char *name=0 );
    ~MakeOptionsWidget();

public slots:
    void accept();

private:
    QDomDocument &m_dom;
    QString m_configGroup;
    EnvironmentVariablesWidget* m_environmentVariablesWidget;
};

#endif
