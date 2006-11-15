/* This file is part of KDevelop
Copyright (C) 2005 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVLANGUAGECONTROLLER_H
#define KDEVLANGUAGECONTROLLER_H

#include <QObject>
#include "kdevcore.h"

#include <QHash>

#include "kdevexport.h"

#include "kdevlanguagesupport.h"

class KDEVPLATFORM_EXPORT KDevLanguageController: public QObject, protected KDevCoreInterface
{
    friend class KDevCore;
    Q_OBJECT
public:
    KDevLanguageController( QObject *parent = 0 );
    virtual ~KDevLanguageController();

    KDevLanguageSupport *activeLanguage() const;
    KDevLanguageSupport *languageSupport( const QString &language );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private:
    bool loadLanguageSupport( const QString &language );

private:
    KDevLanguageSupport *m_activeLanguage;
    QHash<QString, KDevLanguageSupport *> m_languages;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
