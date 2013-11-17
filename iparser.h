/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
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

#ifndef _IPARSER_H_
#define _IPARSER_H_

#include <QXmlStreamReader>

#include "imodel.h"

namespace cppcheck
{

class Parser: public QObject, public QXmlStreamReader
{
    Q_OBJECT

public:

    Parser(QObject* parent = 0) {
        Q_UNUSED(parent);
    };

signals:
    /**
     * Emission of a new item from the parser
     */
    void newElement(cppcheck::Model::eElementType);

    /**
     * Emission of data to register from the parser
     */
    void newData(cppcheck::Model::eElementType, QString name, QString value);

    /**
    * Resets the parser content
    */
    void reset();

    /**
     * Emission of an item from a parser to a model
     */
    void newItem(ModelItem *);

public slots:

    virtual void parse() = 0;

};
}

#endif /* _IMODEL_H_ */
