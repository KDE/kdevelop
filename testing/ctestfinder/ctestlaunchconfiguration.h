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

#ifndef CTESTLAUNCHCONFIGURATION_H
#define CTESTLAUNCHCONFIGURATION_H

#include <interfaces/ilaunchconfiguration.h>
#include <QStringList>
#include <KConfigGroup>

class CTestSuite;

class CTestLaunchConfiguration : public KDevelop::ILaunchConfiguration
{

public:
    CTestLaunchConfiguration(const CTestSuite* suite, const QStringList& cases);
    virtual ~CTestLaunchConfiguration();
    
    virtual KDevelop::IProject* project() const;
    virtual QString name() const;
    virtual KDevelop::LaunchConfigurationType* type() const;
    virtual KConfigGroup config();
    virtual const KConfigGroup config() const;
    
    const CTestSuite* suite() const;
    
private:
    const CTestSuite* m_testSuite;
    QStringList m_cases;
    KConfigGroup m_config;
};

#endif // CTESTLAUNCHCONFIGURATION_H
