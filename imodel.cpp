/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#include "imodel.h"
#include "modelwrapper.h"
#include "job.h"

namespace cppcheck
{

Model::Model()
{
    m_modelWrapper = NULL;
}

void Model::setModelWrapper(ModelWrapper* mdl)
{
    m_modelWrapper = mdl;
}

ModelWrapper* Model::getModelWrapper() const
{
    return m_modelWrapper;
}

Job*    Model::job() const
{
    if (m_modelWrapper != NULL)
        return m_modelWrapper->job();
    return NULL;
}
}
