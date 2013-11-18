/* This file is part of KDevelop
 * Copyright 2011 Sebastien Rannou <mxs@sbrk.org>
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

#ifndef _IVIEW_H_
#define _IVIEW_H_

class QWidget;

namespace cppcheck
{
class Model;

class IView
{
public:
    virtual Model* model(void) = 0;
    virtual void setModel(cppcheck::Model*) = 0;
    virtual ~IView(void) {}

    //get notify that the container size has changed
    virtual void WidgetContainerResizeEvent(QResizeEvent* event) {
        Q_UNUSED(event);
    }
};
}

#endif /* _IVIEW_H_ */
