/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PARTEXPLORERPLUGIN_H_
#define _PARTEXPLORERPLUGIN_H_

#include <qguardedptr.h>

#include "kdevplugin.h"

class QWidget;
class QPainter;
class KURL;
class PartExplorerForm;

class PartExplorerPlugin : public KDevPlugin
{
    Q_OBJECT
public:
    /**
    * Default constructor
    */
    PartExplorerPlugin(  QObject *parent, const char *name, const QStringList & );

    /**
    * Destructor
    */
    virtual ~PartExplorerPlugin();

private slots:
    void slotShowForm();

private:
    /**
    * The dialog for user interaction, so the plugin is usable too ;)
    */
    QGuardedPtr<PartExplorerForm> m_widget;
};

#endif // _PARTEXPLORERPLUGIN_H_
