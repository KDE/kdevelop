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
#include <kurl.h>

class EnvironmentVariablesWidget;


class RunOptionsWidget : public RunOptionsWidgetBase
{
    Q_OBJECT
    
public:
    RunOptionsWidget( QDomDocument &dom,             //!< document DOM
                      const QString &configGroup,    //!< configuration group
                      const QString &buildDirectory, //!< project build directory
                      QWidget *parent=0,             //!< parent widget
                      const char *name=0             //!< widget's name
                       );
    ~RunOptionsWidget();

public slots:
    void accept();

private:
    virtual void directoryRadioChanged();
    virtual void browseCustomDirectory();
    virtual void browseMainProgram();

    QDomDocument &m_dom;
    QString m_configGroup;
    KURL m_buildDirectory;
    KURL m_customRunDirectory;
    KURL m_mainProgramAbsolutePath;
    EnvironmentVariablesWidget* m_environmentVariablesWidget;
};

#endif
