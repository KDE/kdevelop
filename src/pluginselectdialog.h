/***************************************************************************
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PLUGINSELECTDIALOG_H
#define PLUGINSELECTDIALOG_H

#include "pluginselectdialogbase.h"

class PluginController;

class PluginSelectDialog : public PluginSelectDialogBase
{
	Q_OBJECT

public:
	PluginSelectDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~PluginSelectDialog();

	QStringList unselectedPluginNames();

private:
	void init();

protected slots:
	virtual void saveAsDefault();
    void itemSelected( QListViewItem * );
    void openURL( const QString & );

};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
