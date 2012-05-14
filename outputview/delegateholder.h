/*
    This file is part of KDevelop
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DELEGATEHOLDER_H
#define DELEGATEHOLDER_H

#include "outputviewexport.h"

class QObject;

namespace KDevelop
{

class OutputDelegate;

/**
 * This class holds the outputview delegate because the delegate outlives the job that 'sets' it.
 * Inherit from this class if your plugin needs to hold the outputview delegate.
 */
class KDEVPLATFORMOUTPUTVIEW_EXPORT DelegateHolder
{
public:
    DelegateHolder( QObject* parent);
    ~DelegateHolder();
    OutputDelegate* delegate() const;
private:
    OutputDelegate* m_delegate;
};
}
#endif // DELEGATEHOLDER_H
