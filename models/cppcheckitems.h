/* This file is part of KDevelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef _CPPCHECKITEM_H_
#define _CPPCHECKITEM_H_

#include <QString>

#include "imodel.h"

namespace cppcheck
{
class CppcheckItem : public ModelItem
{

public:

    virtual ~CppcheckItem() {}

    virtual CppcheckItem* parent() const {
        return 0L;
    }

    /*
     * Called when data related to the error has been parsed
     */
    virtual void incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity) = 0;
};
}
#endif /* _CPPCHECKITEM_H_ */
