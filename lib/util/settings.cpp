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

#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "settings.h"

QString Settings::terminalEmulatorName( KConfig & config )
{
	config.setGroup("TerminalEmulator");
	bool useKDESetting = config.readBoolEntry( "UseKDESetting", true );
	QString terminal;

	if ( useKDESetting )
	{
		KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
		terminal = confGroup.readEntry("TerminalApplication", QString::fromLatin1("konsole"));
	}
	else
	{
		terminal = config.readEntry( "TerminalApplication", QString::fromLatin1("konsole"));
	}
	return terminal;
}

QString Settings::profileByAttributes(const QString &language, const QStringList &keywords)
{
    KConfig config(locate("data", "kdevelop/profiles/projectprofiles"));
    config.setGroup(language);

    QStringList profileKeywords = QStringList::split("/", "Empty");
    if (config.hasKey("Keywords"))
        profileKeywords = config.readListEntry("Keywords");

    int idx = 0;
    for (QStringList::const_iterator it = profileKeywords.constBegin();
        it != profileKeywords.constEnd(); ++it)
    {
        if (keywords.contains(*it))
        {
            idx = profileKeywords.findIndex(*it);
            break;
        }
    }

    QStringList profiles;
    if (config.hasKey("Profiles"))
    {
        profiles = config.readListEntry("Profiles");
        return profiles[idx];
    }
    return "KDevelop";
}
