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

#ifndef _DOCTREEGLOBALCONFIGWIDGET_H_
#define _DOCTREEGLOBALCONFIGWIDGET_H_

#include "doctreeglobalconfigwidgetbase.h"

class DocTreeViewWidget;


class DocTreeGlobalConfigWidget : public DocTreeGlobalConfigWidgetBase
{
    Q_OBJECT

public:
    DocTreeGlobalConfigWidget( DocTreeViewWidget *widget, QWidget *parent, const char *name=0 );
    ~DocTreeGlobalConfigWidget();

public slots:
    void accept();

private slots:
    void updateIndexClicked();

private:
    void readConfig();
    void storeConfig();
    
    DocTreeViewWidget *m_widget;
};

#endif
