/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEVIEWCONFIGWIDGET_H_
#define _FILEVIEWCONFIGWIDGET_H_

#include "fileviewconfigwidgetbase.h"

class FileViewPart;


class FileViewConfigWidget : public FileViewConfigWidgetBase
{
    Q_OBJECT

public:
    FileViewConfigWidget( FileViewPart *widget, QWidget *parent, const char *name=0 );
    ~FileViewConfigWidget();

public slots:
     void accept();

private:
    virtual void addGroup();
    virtual void removeGroup();
    
    void readConfig();
    void storeConfig();
    
    FileViewPart *m_part;
};

#endif
