/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#ifndef IRUNPROVIDER_H
#define IRUNPROVIDER_H

#include "irun.h"

#include "iextension.h"

namespace KDevelop
{

/**
 * An abstract interface for plugins which are able to execute programs.
 */
class KDEVPLATFORMINTERFACES_EXPORT IRunProvider
{
public:
    virtual ~IRunProvider();

    enum OutputTypes {
        StandardOutput /**< indicates the process wrote to standard output */,
        StandardError  /**< indicates the process wrote to standard error */,
        RunProvider    /**< indicates the run provider produced output */
    };
    
    /**
     * Return a list of instrumentors that your plugin supports.
     */
    virtual QStringList instrumentorsProvided() const = 0;

    /**
     * Request the execution of \a run.
     */
    virtual bool execute(const IRun& run, int serial) = 0;

    /**
     * Request the aborting of a run with the given \a serial number.
     */
    virtual void abort(int serial) = 0;

Q_SIGNALS:
    /**
     * Notify that process with the given \a serial number has finished.
     */
    void finished(int serial);

    /**
     * Notify that the process with given \a serial has produced the given \a output in the given \a type.
     */
    void output(int serial, const QString& line, KDevelop::IRunProvider::OutputTypes type);
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS(KDevelop, IRunProvider, "org.kdevelop.IRunProvider")
Q_DECLARE_INTERFACE(KDevelop::IRunProvider, "org.kdevelop.IRunProvider")

#endif
