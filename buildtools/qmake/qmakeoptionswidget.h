/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QMAKEOPTIONSWIDGET_H
#define QMAKEOPTIONSWIDGET_H

#include "qmakeoptionswidgetbase.h"

#include <qdom.h>

class QMakeOptionsWidget : public QMakeOptionsWidgetBase
{
    Q_OBJECT
public:
    QMakeOptionsWidget( QDomDocument &dom, const QString &configGroup,
                        QWidget *parent = 0, const char *name = 0 );
    ~QMakeOptionsWidget();

public slots:
    void accept();
private:
    QDomDocument &m_dom;
    QString m_configGroup;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on


