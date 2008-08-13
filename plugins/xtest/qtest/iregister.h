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

#ifndef VERITAS_QTEST_IREGISTER_H
#define VERITAS_QTEST_IREGISTER_H

namespace KDevelop { class IProject; }
namespace Veritas { class Test; }

namespace QTest
{

/*! Collects the test tree structure for a project. */
class IRegister
{
public:
    IRegister();
    virtual ~IRegister();

    /*! Discard the previous test tree structure and compute it anew
    @note potentially expensive. should not be called in the gui thread.*/
    virtual void reload() = 0;

    /*! Fetch the cached root. */
    virtual Veritas::Test* root() const = 0;

    /*! This initializes the register with a project to fetch
    tests for.
    @note mandatory to invoke this exactly once */
    void setProject(KDevelop::IProject*);
    KDevelop::IProject* project() const;

private:
    KDevelop::IProject* m_project;
};
}

#endif // VERITAS_QTEST_IREGISTER_H
