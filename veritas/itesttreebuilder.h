/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#ifndef VERITAS_ITESTTREEBUILDER_H
#define VERITAS_ITESTTREEBUILDER_H

#include <QtCore/QObject>
#include "veritasexport.h"

namespace KDevelop { class IProject; }

namespace Veritas
{
class Test;

/*! Collects the test tree structure for a project. */
class VERITAS_EXPORT ITestTreeBuilder : public QObject
{
Q_OBJECT
public:
    ITestTreeBuilder();
    virtual ~ITestTreeBuilder();

    /*! Fetch the cached root. */
    virtual Veritas::Test* root() const = 0;

    /*! Discard the previous test tree structure and recompute it.
     *  Operates asynchronously, reloadFinished() is emitted when done.*/
    virtual void reload(KDevelop::IProject* project) = 0;

Q_SIGNALS:
    /*! Emitted when a reload cycle has completed */
    void reloadFinished(Veritas::Test* root);

    /*! Something went wrong */
    void reloadFailed();
};

}

#endif // VERITAS_ITESTTREEBUILDER_H
