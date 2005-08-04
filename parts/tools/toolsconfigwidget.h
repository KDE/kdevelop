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
#include <q3dict.h>
//Added by qt3to4:
#include <QEvent>


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

    virtual bool eventFilter(QObject *o, QEvent *e);

    void readGroup(const QString &group, Q3Dict<ToolsConfigEntry> *entryDict);
    void storeGroup(const QString &group, const Q3Dict<ToolsConfigEntry> &entryDict);
    void fillListBox(Q3ListBox *lb, const Q3Dict<ToolsConfigEntry> &entryDict);
    bool addEntry(ToolsConfigEntry *entry, Q3Dict<ToolsConfigEntry> *entryDict);

    void readConfig();
    void storeConfig();
    void updateListBoxes();
    
    Q3Dict<ToolsConfigEntry> m_toolsmenuEntries;
    Q3Dict<ToolsConfigEntry> m_filecontextEntries;
    Q3Dict<ToolsConfigEntry> m_dircontextEntries;
    
    ToolsPart *m_part;
};

#endif
