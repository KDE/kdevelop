/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef I_SESSIONLOCK_H
#define I_SESSIONLOCK_H

#include "interfacesexport.h"
#include <QSharedPointer>

class QUuid;

namespace KDevelop {

/**
 * This class ensures that a given session is properly locked and no other
 * KDevplatform process may use this session.
 */
class KDEVPLATFORMINTERFACES_EXPORT ISessionLock
{
public:
    /**
     * Use this shared pointer to pass around the session lock.
     */
    typedef QSharedPointer<ISessionLock> Ptr;

    virtual ~ISessionLock();

    /**
     * @return the unique session ID.
     */
    virtual QUuid id() = 0;
};

}

#endif // I_SESSIONLOCK_H
