/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _RUNOPTIONSWIDGET_H_
#define _RUNOPTIONSWIDGET_H_

#include "runoptionswidgetbase.h"

#include <qdom.h>


class RunOptionsWidget : public RunOptionsWidgetBase
{
    Q_OBJECT
    
public:
    RunOptionsWidget( QDomDocument &dom, const QString &configGroup,
                      QWidget *parent=0, const char *name=0 );
    ~RunOptionsWidget();

public slots:
    void accept();

private:
    virtual void addVarClicked();
    virtual void removeVarClicked();
    virtual void browseMainProgram();
    
    QDomDocument &m_dom;
    QString m_configGroup;
};

#endif
