/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
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

#ifndef _MARKS_H_
#define _MARKS_H_

#include <QObject>

namespace cppcheck
{
class Model;
class Plugin;

/*
 * Interactions between the file editor and Cppcheck's output
 */
class Marks : public QObject
{
    Q_OBJECT

public:

    Marks(cppcheck::Plugin *plugin);
    ~Marks();

private slots:
    /*
     * Sets the new model
     * Clean the previous marks if necessary
     */
    void newModel(cppcheck::Model* model);

    void modelChanged();
private:
    cppcheck::Plugin  *m_plugin;
    cppcheck::Model   *m_model;
};

}



#endif /* _MARKS_H_ */
