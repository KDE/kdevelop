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

#include <QtCore/QObject>

namespace KDevelop { class IProject; }
namespace Veritas { class Test; }

namespace QTest
{
class ISettings;

/*! Collects the test tree structure for a project. */
class IRegister : public QObject
{
Q_OBJECT
public:
    IRegister();
    virtual ~IRegister();

    /*! Discard the previous test tree structure and compute it anew
    @note potentially expensive. should not be called in the gui thread.*/
    virtual void reload() = 0;

    /*! Fetch the cached root. */
    virtual Veritas::Test* root() const = 0;

    /*! Initialize the project to register tests for.
    @note mandatory to invoke this exactly once */
    void setProject(KDevelop::IProject*);
    KDevelop::IProject* project() const;

    /*! Initialize KConfig abstraction
    @note mandatory to invoke this exactly once */
    void setSettings(ISettings*);
    ISettings* settings() const;

Q_SIGNALS:
    /*! Emitted when a reload cycle has completed */
    void reloadFinished(Veritas::Test* root);

    /*! Something went wrong */
    void reloadFailed();

private:
    KDevelop::IProject* m_project;
    ISettings* m_settings;
};
}

#endif // VERITAS_QTEST_IREGISTER_H
