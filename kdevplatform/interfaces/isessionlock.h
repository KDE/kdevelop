/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef I_SESSIONLOCK_H
#define I_SESSIONLOCK_H

#include "interfacesexport.h"
#include <QSharedPointer>

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
    using Ptr = QSharedPointer<ISessionLock>;

    virtual ~ISessionLock();

    /**
     * @return the unique session ID.
     */
    virtual QString id() = 0;
};

}

#endif // I_SESSIONLOCK_H
