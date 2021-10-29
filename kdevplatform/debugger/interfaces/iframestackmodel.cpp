/*
    SPDX-FileCopyrightText: 2009 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iframestackmodel.h"

namespace KDevelop {

class IFrameStackModelPrivate
{
public:
    IDebugSession *m_session = nullptr;
};

IFrameStackModel::IFrameStackModel(KDevelop::IDebugSession* session)
    : QAbstractItemModel(session)
    , d_ptr(new IFrameStackModelPrivate)
{
    Q_D(IFrameStackModel);

    d->m_session = session;
}

IFrameStackModel::~IFrameStackModel()
{
}

IDebugSession* IFrameStackModel::session() const
{
    Q_D(const IFrameStackModel);

    return d->m_session;
}

}
