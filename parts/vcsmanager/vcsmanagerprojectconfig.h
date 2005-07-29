/***************************************************************************
*   Copyright (C) 2005 by Jens Dagerbo   *
*   jens.dagerbo@swipnet.se   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
***************************************************************************/

#ifndef VCSMANAGER_PROJECT_CONFIG_H
#define VCSMANAGER_PROJECT_CONFIG_H

#include <ktrader.h>
#include "vcsmanagerprojectconfigbase.h"

class VCSManagerPart;

class VCSManagerProjectConfig: public VCSManagerProjectConfigBase
{
	Q_OBJECT
public:
	VCSManagerProjectConfig( VCSManagerPart *part, QWidget *parent = 0, const char *name = 0);

public slots:
	void accept();

private:
	void setup();

	VCSManagerPart * m_part;
	QStringList m_vcsPluginNames;
};

#endif
