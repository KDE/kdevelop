/***************************************************************************
 *   Copyright (C) 2005 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CTAGS2_SETTINGSWIDGET_H
#define CTAGS2_SETTINGSWIDGET_H

#include "ctags2_settingswidgetbase.h"

class CTags2Part;

class CTags2SettingsWidget : public CTags2SettingsWidgetBase
{
	Q_OBJECT

public:
	CTags2SettingsWidget( CTags2Part * part, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
	~CTags2SettingsWidget();

signals:
	void newTagsfileName( const QString & name );

public slots:
	void slotAccept();
	
protected:

protected slots:

private:
	void loadSettings();
	void storeSettings();
	
	CTags2Part * m_part;

};

#endif

