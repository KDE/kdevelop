/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_ISESSION_H
#define KDEVPLATFORM_ISESSION_H

#include "interfacesexport.h"
#include <QtCore/QObject>

#include <ksharedconfig.h>
#include <kurl.h>

struct QUuid;
class QString;
class KUrl;

namespace KDevelop
{

class IPlugin;

/**
 * @class ISession
 */
class KDEVPLATFORMINTERFACES_EXPORT ISession : public QObject
{
    Q_OBJECT
public:
    ISession( QObject* parent = 0 );
    virtual ~ISession();

    /**
     * A short string nicely identifying the session, including contained projects
     * 
     * The string is empty if the session is empty and has no name.
     */
    virtual QString description() const = 0;
    virtual QString name() const = 0;
    virtual KUrl::List containedProjects() const = 0;
    virtual void updateContainedProjects() = 0;
    virtual KUrl pluginDataArea( const IPlugin* ) = 0;
    virtual KSharedConfig::Ptr config() = 0;
    virtual QUuid id() const = 0;

    /**
     * Mark session as temporary. It will then be deleted on close.
     *
     * This is mainly useful for unit tests etc.
     */
    virtual void setTemporary(bool temp) = 0;
    virtual bool isTemporary() const = 0;
};

}

#endif

