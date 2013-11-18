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

#ifndef _MODELWRAPPER_H_
#define _MODELWRAPPER_H_

#include "imodel.h"

namespace cppcheck
{

/**
 * Allow sending signal from nested classes
 */
class ModelEvents
{
public:
    virtual ~ModelEvents();
    virtual void modelChanged() = 0;
};

class ModelInterface;

/**
 * This class is used to wrap signal
 * It has been created because there was problems with multiple QObject inheritence
 * Before Model inherit QAbstractItemModel but there was problem when a tool has many model
 * That why this class exists
 */
class ModelWrapper :  public QObject,
                      public ModelEvents
{
  Q_OBJECT
public:

    ModelWrapper(Model *impl);

    ~ModelWrapper();

public slots:

    /**
     * Compatibility with cppcheck module, TO REMOVE
     * Reception of a new item in the model
     */
    void newElement(cppcheck::Model::eElementType);

    /**
     * Compatibility with cppcheck module, to remove
     * Reception of a new item in the model
     * Has to be pure
     */
    void newItem(cppcheck::ModelItem *);

    /**
     * Reception of data to register to the current item
     * TO REMOVE
     */
    void newData(cppcheck::Model::eElementType, QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    /**
     * Resets the model content
     */
    void reset();

    /**
     * Set the associated process
     */
    void job(cppcheck::Job * job);

    /**
     * Get the associated process
     */
    cppcheck::Job * job(void);

    void jobDestroyed(void);

signals:

    /**
     * emit this signal to alert other modules that the model has been updated
     */
    void modelChanged();

private:
    cppcheck::Job *m_job;
    Model         *m_modelImplementation;
};
}
#endif
