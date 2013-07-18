/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_HOLDUPDATES_H
#define KDEVPLATFORM_HOLDUPDATES_H

#include "sublimeexport.h"

class QWidget;

namespace Sublime
{
/**
This class makes it possible to have nested scoped calls to QWidget::setUpdatesEnabled.

For that matter: 

@code 
{
    ...
    setUpdatesEnabled(false);
    someCall();
    setUpdatesEnabled(true);
    ...
}
@endcode

Would be equivalent to 

@code
{
    ...
    HoldUpdates hu(this);
    someCall();
    hu.stop();//if we need to, otherwise we can let it fall out of scope
    ...
}
@endcode

if someCall was using HoldUpdates too, it will make sure to only call setUpdatesEnabled(true) in case it's needed.

 */
class SUBLIME_EXPORT HoldUpdates {
    public:
        HoldUpdates(QWidget* w);
        ~HoldUpdates();
         void stop();
    
    private:
        bool m_wasupdating;
        bool m_done;
        QWidget* m_w;
};

}

#endif // KDEVPLATFORM_HOLDUPDATES_H

