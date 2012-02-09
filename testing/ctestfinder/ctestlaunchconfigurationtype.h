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

#ifndef CTESTLAUNCHCONFIGURATIONTYPE_H
#define CTESTLAUNCHCONFIGURATIONTYPE_H

#include "interfaces/launchconfigurationtype.h"


class CTestLaunchConfigurationType : public KDevelop::LaunchConfigurationType
{

public:
    CTestLaunchConfigurationType();
    virtual ~CTestLaunchConfigurationType();
    
    virtual bool canLaunch(const KUrl& file) const;
    virtual void configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const;
    virtual void configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const;
    virtual bool canLaunch(KDevelop::ProjectBaseItem* item) const;
    virtual KIcon icon() const;
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const;
    virtual QString name() const;
    virtual QString id() const;
};

#endif // CTESTLAUNCHCONFIGURATIONTYPE_H
