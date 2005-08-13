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

#ifndef _CUSTOMBUILDOPTIONSWIDGET_H_
#define _CUSTOMBUILDOPTIONSWIDGET_H_

#include "custombuildoptionswidgetbase.h"
#include <qdom.h>

class QTabWidget;


class CustomBuildOptionsWidget : public CustomBuildOptionsWidgetBase
{
    Q_OBJECT
    
public:
    CustomBuildOptionsWidget( QDomDocument &dom, QWidget *parent=0, const char *name=0 );
    ~CustomBuildOptionsWidget();

    void setMakeOptionsWidget(QTabWidget *tw, QWidget *mow);
    
public slots:
    void accept();

private:
    virtual void makeToggled(bool b);
    
    QDomDocument &m_dom;
    QTabWidget *m_tabWidget;
    QWidget *m_makeOptions;
};

#endif
