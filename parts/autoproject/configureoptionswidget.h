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

#ifndef _CONFIGUREOPTIONSWIDGET_H_
#define _CONFIGUREOPTIONSWIDGET_H_

#include "domutil.h"
#include "configureoptionswidgetbase.h"

class KDevCompilerOptions;
class AutoProjectPart;


class ConfigureOptionsWidget : public ConfigureOptionsWidgetBase
{
    Q_OBJECT
    
public:
    ConfigureOptionsWidget( AutoProjectPart *part, QWidget *parent=0, const char *name=0 );
    ~ConfigureOptionsWidget();

public slots:
    void accept();

private:
    virtual void cflagsClicked();
    virtual void cxxflagsClicked();
    virtual void f77flagsClicked();
    virtual void cserviceChanged();
    virtual void cxxserviceChanged();
    virtual void f77serviceChanged();

    KDevCompilerOptions *createCompilerOptions(const QString &lang);
    QStringList cservice_names, cservice_execs;
    QStringList cxxservice_names, cxxservice_execs;
    QStringList f77service_names, f77service_execs;
    
    AutoProjectPart *m_part;
};

#endif
