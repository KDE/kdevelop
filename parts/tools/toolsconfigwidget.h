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

#ifndef _TOOLSCONFIGWIDGET_H_
#define _TOOLSCONFIGWIDGET_H_

#include "toolsconfigwidgetbase.h"
#include "tools_part.h"


struct ToolsConfigEntry;


class ToolsConfigWidget : public ToolsConfigWidgetBase
{
    Q_OBJECT
    
public:
    ToolsConfigWidget(QWidget *parent=0, const char *name=0);
    ~ToolsConfigWidget();

public slots:
    void accept();
    
private:
    virtual void toolsmenuaddClicked();
    virtual void toolsmenuremoveClicked();
    virtual void filecontextaddClicked();
    virtual void filecontextremoveClicked();
    virtual void dircontextaddClicked();
    virtual void dircontextremoveClicked();

    void readConfig();
    void storeConfig();
    void updateListBoxes();
    
    QList<ToolsConfigEntry> m_toolsmenuEntries;
    QList<ToolsConfigEntry> m_filecontextEntries;
    QList<ToolsConfigEntry> m_dircontextEntries;
    
    ToolsPart *m_part;
};

#endif
