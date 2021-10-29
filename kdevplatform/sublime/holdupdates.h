/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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
class KDEVPLATFORMSUBLIME_EXPORT HoldUpdates {
    public:
        explicit HoldUpdates(QWidget* w);
        ~HoldUpdates();
         void stop();
    
    private:
        bool m_wasupdating;
        bool m_done;
        QWidget* const m_w;
};

}

#endif // KDEVPLATFORM_HOLDUPDATES_H

