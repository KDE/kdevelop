/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "main.h"
#include "cprintdlg.h"


extern "C" {

    void *init_libkdevprintplugin()
    {
        return new PrintFactory;
    }
    
};


PrintFactory::PrintFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


PrintFactory::~PrintFactory()
{}


QObject *PrintFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{
    QString filename;
    if (!args.isEmpty())
        filename = *args.begin();

    if (!parent->isWidgetType()) {
        qDebug("Parent of print dialog is not a widget");
        return 0;
    }
    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog
    
    QObject *obj = new CPrintDlg(parentWidget, filename, name, false);
    emit objectCreated(obj);
    return obj;
}
#include "main.moc"
