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

#include "misc.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>

void AppWizardUtil::guessAuthorAndEmail(QString *author, QString *email)
{
    char hostname[512];

    QString cfgName(KGlobal::dirs()->findResource("config","emaildefaults"));
    if(cfgName.isEmpty()) {
        struct passwd *pw = ::getpwuid(getuid());
        // pw==0 => the system must be really fucked up
        if (!pw)
            return;

        // I guess we don't have to support users with longer host names ;-)
        (void) ::gethostname(hostname, sizeof hostname);

        *author = QString::fromLocal8Bit(pw->pw_gecos);
        *email = QString(pw->pw_name) + "@" + hostname;
    }
    else {
	KConfig cfg(cfgName, true);
	cfg.setGroup("Defaults");
	QString profile = cfg.readEntry("Profile", "Default");    
        cfg.setGroup("PROFILE_" + profile);
	*author = cfg.readEntry("FullName", "Author");
        *email = cfg.readEntry("EmailAddress", "me@me.com");	
    }
}
