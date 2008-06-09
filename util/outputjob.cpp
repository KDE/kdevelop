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

#include "outputjob.h"

#include <QStandardItemModel>
#include <QItemDelegate>

#include "icore.h"
#include "iplugincontroller.h"
#include "ioutputview.h"

using namespace KDevelop;

OutputJob::OutputJob(QObject* parent)
    : KJob(parent)
    , m_standardToolView(-1)
    , m_type(IOutputView::OneView)
    , m_behaviours(IOutputView::AllowUserClose)
    , m_killJobOnOutputClose(true)
    , m_raiseOnCreation(true)
    , m_outputId(-1)
    , m_outputModel(0)
    , m_modelOwnership(IOutputView::KeepOwnership)
    , m_outputDelegate(0)
    , m_delegateOwnership(IOutputView::KeepOwnership)
{
}

void OutputJob::startOutput()
{
    IPlugin* i = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            int tvid;
            if (m_standardToolView != -1) {
                tvid = view->standardToolView( static_cast<IOutputView::StandardToolView>(m_standardToolView) );
            } else {
                tvid = view->registerToolView(m_toolTitle, m_type);
            }

            m_outputId = view->registerOutputInToolView( tvid, m_title, m_behaviours );

            if (!m_outputModel) {
                m_outputModel = new QStandardItemModel(0);
                m_modelOwnership = IOutputView::TakeOwnership;
            }

            // Keep the item model around after the job is gone
            view->setModel(m_outputId, m_outputModel, m_modelOwnership);

            if (!m_outputDelegate) {
                m_outputDelegate = new QItemDelegate(0);
                m_delegateOwnership = IOutputView::TakeOwnership;
            }

            view->setDelegate(m_outputId, m_outputDelegate, m_delegateOwnership);

            if (m_killJobOnOutputClose)
                connect(i, SIGNAL(outputRemoved(int, int)), this, SLOT(outputViewRemoved(int, int)));

            if (m_raiseOnCreation)
                view->raiseOutput(m_outputId);
        }
    }
}

void OutputJob::outputViewRemoved(int , int id)
{
    if (id == m_outputId && m_killJobOnOutputClose)
        kill();
}

void KDevelop::OutputJob::setTitle(const QString & title)
{
    m_title = title;
}

void KDevelop::OutputJob::setViewType(IOutputView::ViewType type)
{
    m_type = type;
}

void KDevelop::OutputJob::setBehaviours(IOutputView::Behaviours behaviours)
{
    m_behaviours = behaviours;
}

void KDevelop::OutputJob::setKillJobOnOutputClose(bool killJobOnOutputClose)
{
    m_killJobOnOutputClose = killJobOnOutputClose;
}

void KDevelop::OutputJob::setModel(QStandardItemModel * model, IOutputView::Ownership takeOwnership)
{
    m_modelOwnership = takeOwnership;
    m_outputModel = model;
}

void KDevelop::OutputJob::setDelegate(QAbstractItemDelegate * delegate, IOutputView::Ownership takeOwnership)
{
    m_delegateOwnership = takeOwnership;
    m_outputDelegate = delegate;
}

QStandardItemModel * KDevelop::OutputJob::model() const
{
    return m_outputModel;
}

void KDevelop::OutputJob::setStandardToolView(IOutputView::StandardToolView standard)
{
    m_standardToolView = standard;
}

void KDevelop::OutputJob::setRaiseOnCreation(bool raise)
{
    m_raiseOnCreation = raise;
}

void OutputJob::setToolTitle(const QString& title)
{
    m_toolTitle = title;
}

#include "outputjob.moc"
