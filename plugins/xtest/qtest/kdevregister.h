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

#ifndef VERITAS_QTEST_KDEVREGISTER_H
#define VERITAS_QTEST_KDEVREGISTER_H

#include "qxqtestexport.h"
#include "iregister.h"
#include <QList>

namespace Veritas { class Test; }
namespace KDevelop { class ProjectTestTargetItem; }

namespace QTest
{

/*! @unittest QTest::Test::KDevRegisterTest */
class QXQTEST_EXPORT KDevRegister : public IRegister
{
public:
    KDevRegister();
    virtual ~KDevRegister();

    virtual void reload();
    virtual Veritas::Test* root() const;

protected:
    /*! protected to allow this to be stubbed out in tests
    @note this is an expensive operation since it involves searching
    for files. */
    virtual QList<KDevelop::ProjectTestTargetItem*> fetchTestTargets();

private:
    Veritas::Test* m_root;
};

}

#endif // VERITAS_QTEST_KDEVREGISTER_H
