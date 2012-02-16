/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef CTESTFINDER_H
#define CTESTFINDER_H

#include "ictestprovider.h"
#include <interfaces/iplugin.h>
#include <interfaces/itestprovider.h>

class KJob;
class CTestSuite;
class QVariant;
class CTestLaunchConfigurationType;

namespace KDevelop
{
    class IProject;
    class ITestController;
}

class CTestFinder : public KDevelop::IPlugin, public KDevelop::ITestProvider, public ICTestProvider
{
   Q_OBJECT
   Q_INTERFACES(KDevelop::ITestProvider)
   Q_INTERFACES(ICTestProvider)
public:
    CTestFinder(QObject* parent, const QList<QVariant>& args);
    virtual ~CTestFinder();
    virtual void unload();

    virtual void createTestSuite(const QString& name, const QString& executable, KDevelop::IProject* project, const QStringList& arguments = QStringList());
    virtual KJob* findTests();

private:
    QList<CTestSuite*> m_pendingSuites;
};

#endif // CTESTFINDER_H
