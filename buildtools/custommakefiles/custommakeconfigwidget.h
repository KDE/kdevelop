/***************************************************************************
 *   Copyright (C) 2003 by Hendrik Kueck                                   *
 *   kueck@cs.ubc.ca                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef CUSTOMMAKECONFIGWIDGET_H
#define CUSTOMMAKECONFIGWIDGET_H

#include "domutil.h"

#include <custommakeconfigwidgetbase.h>

class CustomProjectPart;
class EnvironmentVariablesWidget;

/**
@author KDevelop Authors
*/
class CustomMakeConfigWidget : public CustomMakeConfigWidgetBase
{
    Q_OBJECT

public:
    CustomMakeConfigWidget(CustomProjectPart* part, const QString& configGroup, QWidget* parent);

    ~CustomMakeConfigWidget();

public slots:
    void accept();

protected:
    CustomProjectPart* m_part;
    QString m_configGroup;
    QDomDocument& m_dom;

    QStringList m_allEnvironments;
    QString m_currentEnvironment;

    EnvironmentVariablesWidget* m_envWidget;

    virtual void envNameChanged(const QString& envName);
    virtual void envChanged(const QString& envName);
    virtual void envAdded();
    virtual void envRemoved();
    virtual void envCopied();

};

#endif
