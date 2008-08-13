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

#include "iregister.h"
#include <interfaces/iproject.h>
#include <Qt>

using KDevelop::IProject;
using QTest::IRegister;

IRegister::IRegister() : m_project(0)
{}

IRegister::~IRegister()
{}

void IRegister::setProject(IProject* project)
{
    Q_ASSERT(project); Q_ASSERT(m_project == 0);
    m_project = project;
}

IProject* IRegister::project() const
{
    Q_ASSERT(m_project);
    return m_project;
}
