/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IQMAKEBUILDER_H
#define IQMAKEBUILDER_H

#include <project/interfaces/iprojectbuilder.h>

namespace KDevelop {
class IProject;
}

/**
@author Andreas Pakulat
*/

class IQMakeBuilder : public KDevelop::IProjectBuilder
{
public:
    ~IQMakeBuilder() override = default;

};

Q_DECLARE_INTERFACE( IQMakeBuilder, "org.kdevelop.IQMakeBuilder" )

#endif
