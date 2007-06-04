/***************************************************************************
                                settingsbase.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/
 
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

#include <qobject.h>

#include "kompare.h"

class QWidget;
class KConfig;

class SettingsBase : public QObject
{
Q_OBJECT
public:
	SettingsBase( QWidget* parent );
	~SettingsBase();

public:
	virtual void loadSettings( KConfig* config );
	virtual void saveSettings( KConfig* config );
};

#endif
