/* This file is part of KDevelop
 * Copyright 2011 Lucas SARIE <lucas.sarie@gmail.com>
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

#include "modelwrapper.h"

#include "imodel.h"
#include "job.h"

namespace cppcheck
{

ModelWrapper::ModelWrapper(Model *impl)
    : m_job(0)
{
    Q_ASSERT(impl);
    m_modelImplementation = impl;
}



ModelWrapper::~ModelWrapper()
{
}

void ModelWrapper::job(cppcheck::Job * job)
{
    m_job = job;
}

cppcheck::Job * ModelWrapper::job(void)
{
    return m_job;
}

void ModelWrapper::jobDestroyed(void)
{
    m_job = 0;
}

void ModelWrapper::newElement(Model::eElementType type)
{
    m_modelImplementation->newElement(type);
}

void ModelWrapper::newData(Model::eElementType type, QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    m_modelImplementation->newData(type, name, value, ErrorLine, ErrorFile, Message, MessageVerbose, ProjectPath, Severity);
}

void ModelWrapper::newItem(ModelItem *item)
{
    m_modelImplementation->newItem(item);
}

void ModelWrapper::reset()
{
    m_modelImplementation->reset();
}

ModelEvents::~ModelEvents()
{
}


}
