/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PROJECTCONFIGURATIONWIDGET_H
#define PROJECTCONFIGURATIONWIDGET_H

#include <qwidget.h>
#include <qvaluelist.h>

#include "kdevbuildsystem.h"

class ProjectConfigurationWidget : public QWidget
{
Q_OBJECT
public:
    ProjectConfigurationWidget(QWidget *parent = 0, const char *name = 0, WFlags f = 0);

    ~ProjectConfigurationWidget();

    void addConfigWidget( ProjectConfigTab* w );
    void addConfigWidgets( QValueList<ProjectConfigTab*> );

public slots:
    void accept();

private:
    QValueList<ProjectConfigTab*> m_configWidgets;
};

#endif
