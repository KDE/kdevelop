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

#ifndef _FILEGROUPSPART_H_
#define _FILEGROUPSPART_H_

#include <qguardedptr.h>
//#include <kdialogbase.h>
#include <configwidgetproxy.h>
#include "kdevplugin.h"

class FileGroupsWidget;
class KDialogBase;

class FileGroupsPart : public KDevPlugin
{
    Q_OBJECT

public:
    FileGroupsPart( QObject *parent, const char *name, const QStringList & );
    ~FileGroupsPart();

public slots:
    void refresh();

private slots:
	void insertConfigWidget( const KDialogBase* dlg, QWidget * page, unsigned int );

private:
    QGuardedPtr<FileGroupsWidget> m_filegroups;
    bool deleteRequested;
	ConfigWidgetProxy * _configProxy;
};

#endif
