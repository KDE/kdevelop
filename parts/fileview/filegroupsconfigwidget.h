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

#ifndef _FILEGROUPSCONFIGWIDGET_H_
#define _FILEGROUPSCONFIGWIDGET_H_

#include "filegroupsconfigwidgetbase.h"

class FileViewPart;


class FileGroupsConfigWidget : public FileGroupsConfigWidgetBase
{
    Q_OBJECT

public:
    FileGroupsConfigWidget( FileViewPart *widget, QWidget *parent, const char *name=0 );
    ~FileGroupsConfigWidget();

public slots:
     void accept();

private:
    virtual void addGroup();
    virtual void removeGroup();
    virtual void moveUp();
    virtual void moveDown();
    
    void readConfig();
    void storeConfig();
    
    FileViewPart *m_part;
};

#endif
