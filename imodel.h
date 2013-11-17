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

#ifndef _IMODEL_H_
#define _IMODEL_H_

#include <QObject>
#include <QAbstractItemModel>

class job;

namespace cppcheck
{

class Job;
class ModelWrapper;

class ModelItem
{

public:
    ModelItem() {};

    virtual ~ModelItem() {};
};

class Model
{
public:
    Model();
    virtual ~Model() {}

    enum eElementType {
        startError,
        error,
        startFrame,
        frame,
        startStack,
        stack
    };

    virtual QAbstractItemModel  *getQAbstractItemModel(int n = 0) = 0;

    virtual void newElement(cppcheck::Model::eElementType) {}
    virtual void newItem(ModelItem *) {}
    virtual void newData(cppcheck::Model::eElementType, QString, QString, int, QString, QString, QString, QString, QString ) {}
    virtual void reset() {};

    void          setModelWrapper(ModelWrapper *mdlw);
    ModelWrapper  *getModelWrapper() const;
    Job           *job() const;
protected:
    ModelWrapper  *m_modelWrapper;
};

}

#endif /* _IMODEL_H_ */
