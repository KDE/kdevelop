/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 John Firebaugh                                     *
 *   jfirebaugh@kde.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ENVIRONMENTVARIABLESWIDGET_H_
#define _ENVIRONMENTVARIABLESWIDGET_H_

#include "environmentvariableswidgetbase.h"

#include <qdom.h>


class EnvironmentVariablesWidget : public EnvironmentVariablesWidgetBase
{
    Q_OBJECT

public:
    EnvironmentVariablesWidget( QDomDocument &dom, const QString &configGroup,
                       QWidget *parent=0, const char *name=0 );
    ~EnvironmentVariablesWidget();

public slots:
    void accept();

private:
    virtual void addVarClicked();
    virtual void removeVarClicked();

    QDomDocument &m_dom;
    QString m_configGroup;
};

#endif
