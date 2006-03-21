/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MISC_H_
#define _MISC_H_

class QString;

class AppWizardUtil
{
public:
    static void guessAuthorAndEmail(QString *author, QString *email);
    static QString kdevRoot(const QString &templateName);
};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
