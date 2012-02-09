/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#ifndef CTESTLAUNCHER_H
#define CTESTLAUNCHER_H

#include <interfaces/ilauncher.h>

class CTestLauncher : public KDevelop::ILauncher
{

public:
    virtual KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg);
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const;
    virtual QStringList supportedModes() const;
    virtual QString description() const;
    virtual QString name() const;
    virtual QString id();
};

#endif // CTESTLAUNCHER_H
