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

#ifndef _CSHARPCONFIGWIDGET_H_
#define _CSHARPCONFIGWIDGET_H_

#include "csharpconfigwidgetbase.h"
#include <qdom.h>


class CSharpConfigWidget : public CSharpConfigWidgetBase
{ 
    Q_OBJECT

public:
    CSharpConfigWidget( QDomDocument &projectDom, QWidget *parent=0, const char *name=0 );
    ~CSharpConfigWidget();

public slots:
    void accept();

private:
    QDomDocument &dom;
};

#endif
