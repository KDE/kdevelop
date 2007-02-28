/***************************************************************************
 *   Copyright (C) 2005 by Achim Herwig                                    *
 *   achim.herwig@wodca.de                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef CUSTOMOTHERCONFIGWIDGET_H
#define CUSTOMOTHERCONFIGWIDGET_H

#include "domutil.h"

#include <customotherconfigwidgetbase.h>

class CustomProjectPart;
class EnvironmentVariablesWidget;

/**
@author KDevelop Authors
*/
class CustomOtherConfigWidget : public CustomOtherConfigWidgetBase
{
    Q_OBJECT

public:
    CustomOtherConfigWidget(CustomProjectPart* part, const QString& configGroup, QWidget* parent);

    ~CustomOtherConfigWidget();

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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

