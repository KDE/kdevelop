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

#ifndef KDEVCMODULE_H
#define KDEVCMODULE_H

#include <QPointer>

#include <kcmodule.h>
#include "kdevexport.h"
#include "kdevconfigskeleton.h"

class KUrl;

/**
 * \short The base class for all KCModule's used by the platform.
 *
 * The base class settings.
 */
class KDEVINTERFACES_EXPORT KDevCModule: public KCModule
{
    Q_OBJECT
public:
    KDevCModule( KDevConfigSkeleton *config,
                 KInstance *instance,
                 QWidget *parent = 0,
                 const QStringList &args = QStringList() );
    virtual ~KDevCModule();

    virtual void save();
    virtual void load();

  /**
   * KCModule's which inherit this class must implement this function and
   * return the url of the kcm's data.kdev4 file installed with the plugin
   * in the plugin's data dir.
   * @return the full url to the installed data.kdev4 of the plugin
   */
    virtual KUrl localNonShareableFile() const = 0;

private:
    QPointer<KDevConfigSkeleton> m_config;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
