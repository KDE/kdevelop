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


void AppWizardUtil::guessAuthorAndEmail(QString *author, QString *email)
{
    char hostname[512];

    struct passwd *pw = ::getpwuid(getuid());
    // pw==0 => the system must be really fucked up
    if (!pw)
        return;

    // I guess we don't have to support users with longer host names ;-)
    (void) ::gethostname(hostname, sizeof hostname);

    *author = pw->pw_gecos;
    *email = QString(pw->pw_name) + "@" + hostname;
}
