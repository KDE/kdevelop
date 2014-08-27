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

#include "interfaces/icore.h"
#include "interfaces/iplugincontroller.h"
#include "outputview/ioutputview.h"

using namespace KDevelop;

OutputJob::OutputJob(QObject* parent, OutputJobVerbosity verbosity)
    : KJob(parent)
    , m_standardToolView(-1)
    , m_type(IOutputView::OneView)
    , m_behaviours(IOutputView::AllowUserClose)
    , m_killJobOnOutputClose(true)
    , m_verbosity(verbosity)
    , m_outputId(-1)
    , m_outputDelegate(0)
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
                tvid = view->registerToolView(m_toolTitle, m_type, m_toolIcon);
            }

            if (m_title.isEmpty())
                m_title = objectName();

            m_outputId = view->registerOutputInToolView( tvid, m_title, m_behaviours );

            if (!m_outputModel) {
                m_outputModel = new QStandardItemModel(0);
            }

            // Keep the item model around after the job is gone
            view->setModel(m_outputId, m_outputModel);

            if (!m_outputDelegate) {
                m_outputDelegate = new QItemDelegate(0);
            }

            view->setDelegate(m_outputId, m_outputDelegate);

            if (m_killJobOnOutputClose)
                connect(i, SIGNAL(outputRemoved(int,int)), this, SLOT(outputViewRemoved(int,int)));

            if (m_verbosity == OutputJob::Verbose)
                view->raiseOutput(m_outputId);
        }
    }
}

void OutputJob::outputViewRemoved(int toolViewId, int id)
{
    Q_UNUSED(toolViewId);
    if (id == m_outputId && m_killJobOnOutputClose)
    {
        // Make sure that the job emits result signal as the job
        // might be used in composite jobs and that one depends
        // on result being emitted to know whether a subjob
        // is done.
        kill( KJob::EmitResult );
    }
}

void KDevelop::OutputJob::setTitle(const QString & title)
{
    m_title = title;
    if (m_outputId >= 0 && m_standardToolView >= 0) {
        IPlugin* i = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
        if( i )
        {
            KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
            if( view )
            {
                view->setTitle(m_outputId, title);
            }
        }
    }
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

void KDevelop::OutputJob::setModel(QAbstractItemModel * model)
{
    m_outputModel = model;
}

void KDevelop::OutputJob::setDelegate(QAbstractItemDelegate * delegate)
{
    m_outputDelegate = delegate;
}

QAbstractItemModel * KDevelop::OutputJob::model() const
{
    return m_outputModel;
}

void KDevelop::OutputJob::setStandardToolView(IOutputView::StandardToolView standard)
{
    m_standardToolView = standard;
}

void OutputJob::setToolTitle(const QString& title)
{
    m_toolTitle = title;
}

void OutputJob::setToolIcon(const QIcon& icon)
{
    m_toolIcon = icon;
}

int OutputJob::outputId() const
{
    return m_outputId;
}

OutputJob::OutputJobVerbosity OutputJob::verbosity() const
{
    return m_verbosity;
}

void OutputJob::setVerbosity(OutputJob::OutputJobVerbosity verbosity)
{
    m_verbosity = verbosity;
}

