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
#include <qdict.h>


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

    void readGroup(const QString &group, QDict<ToolsConfigEntry> *entryDict);
    void storeGroup(const QString &group, const QDict<ToolsConfigEntry> &entryDict);
    void fillListBox(QListBox *lb, const QDict<ToolsConfigEntry> &entryDict);
    bool addEntry(ToolsConfigEntry *entry, QDict<ToolsConfigEntry> *entryDict);

    void readConfig();
    void storeConfig();
    void updateListBoxes();
    
    QDict<ToolsConfigEntry> m_toolsmenuEntries;
    QDict<ToolsConfigEntry> m_filecontextEntries;
    QDict<ToolsConfigEntry> m_dircontextEntries;
    
    ToolsPart *m_part;
};

#endif
