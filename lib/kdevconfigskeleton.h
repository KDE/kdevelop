/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef KDECONFIGSKELETON_H
#define KDECONFIGSKELETON_H

#include "kdevexport.h"
#include <kconfigskeleton.h>

class KDEVPLATFORM_EXPORT KDevConfigSkeleton: public KConfigSkeleton
{
    Q_OBJECT
public:

    KDevConfigSkeleton( const QString & configname = QString() );

    KDevConfigSkeleton( KSharedConfig::Ptr config );

    virtual ~KDevConfigSkeleton();

    virtual void usrWriteConfig();

  /**
   * Every KDevCModule will call this function with the url of the data.kdev4 file
   * installed with the plugin in the plugin's data directory.  The url is determined
   * by the reimplementation of the pure virtual function localNonShareableFile in
   * KDevCModule.
   * The format of the data.kdev4 file is a simple text file filled with a list of names
   * of the settings that are not safe for the global kdev4 project file with each setting
   * name on a different line.  The names are taken from the entry tags of the *.kcfg file.
   * @param url is the full url to the data.kdev4 of the plugin
   */
    void parseNonShareableFile( const KUrl &url );

private:
    bool m_parsed;
    QList<QString> m_nonShareable;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
