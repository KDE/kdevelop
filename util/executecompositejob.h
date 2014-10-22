/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_EXECUTECOMPOSITEJOB_H

#include <kcompositejob.h>
#include "utilexport.h"

template <typename T> class QList;

namespace KDevelop
{

class KDEVPLATFORMUTIL_EXPORT ExecuteCompositeJob : public KCompositeJob
{
Q_OBJECT
public:
    ExecuteCompositeJob(QObject* parent = 0, const QList<KJob*>& jobs = {});
    ~ExecuteCompositeJob();

    virtual void start();
    void setAbortOnError(bool abort);

public Q_SLOTS:
    virtual void slotResult(KJob* job);

protected:
    virtual bool doKill();

private:
    class ExecuteCompositeJobPrivate* const d;
};

}

#endif
