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

#ifndef CTESTLAUNCHER_H
#define CTESTLAUNCHER_H

#include <interfaces/ilauncher.h>

class CTestFinder;

class CTestLauncher : public KDevelop::ILauncher
{
public:
    CTestLauncher(CTestFinder* plugin);
    virtual ~CTestLauncher();
    
public:
    virtual KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg);
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const;
    virtual QStringList supportedModes() const;
    virtual QString description() const;
    virtual QString name() const;
    virtual QString id();
    
private:
    CTestFinder* m_plugin;
};

#endif // CTESTLAUNCHER_H
