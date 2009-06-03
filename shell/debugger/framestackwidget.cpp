/*
 * This file is part of KDevelop
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "framestackwidget.h"

#include <KDebug>
#include <KLocalizedString>
#include <KIcon>

#include "../debugcontroller.h"
#include <debugger/util/treeview.h>
#include <debugger/interfaces/stackmodel.h>
#include <debugger/interfaces/stackitem.h>
#include <QHeaderView>
#include <QScrollBar>
#include <QListView>
#include <QVBoxLayout>
#include <QLabel>


using namespace KDevelop;

FramestackWidget::FramestackWidget(DebugController* controller, QWidget* parent)
    : QSplitter(Qt::Horizontal, parent), m_controller(controller)
{
    connect(controller, SIGNAL(sessionAdded(IDebugSession*)), SLOT(sessionAdded(IDebugSession*)));
    connect(controller, SIGNAL(raiseFramestackViews()), SIGNAL(requestRaise()));
    

    setWhatsThis(i18n("<b>Frame stack</b><p>"
                    "Often referred to as the \"call stack\", "
                    "this is a list showing which function is "
                    "currently active, and what called each "
                    "function to get to this point in your "
                    "program. By clicking on an item you "
                    "can see the values in any of the "
                    "previous calling functions.</p>"));
    setWindowIcon(KIcon("view-list-text"));
    mThreadsWidget=new QWidget(this);
    mThreads=new QListView(mThreadsWidget);
    mFrames=new AsyncTreeView(0, this);
    
    mThreadsWidget->setLayout(new QVBoxLayout());
    mThreadsWidget->layout()->addWidget(new QLabel(i18n("Threads:")));
    mThreadsWidget->layout()->addWidget(mThreads);
    mThreadsWidget->hide();
    addWidget(mThreadsWidget);
    addWidget(mFrames);
    
    setStretchFactor(1, 3);
    connect(mFrames->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(checkFetchMoreFrames()));
    mFrames->setRootIsDecorated(false);
    connect(mThreads, SIGNAL(clicked(QModelIndex)), this, SLOT(setThreadShown(QModelIndex)));
}

FramestackWidget::~FramestackWidget() {}

void FramestackWidget::sessionAdded(KDevelop::IDebugSession* session)
{
    Q_UNUSED(session);
    kDebug() << "Adding session:" << isVisible();
    if (isVisible()) {
        KDevelop::IDebugSession* session = m_controller->currentSession();
        showEvent(0);
    }
}

void KDevelop::FramestackWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    kDebug();
    if (m_controller->currentSession()) {
        m_controller->currentSession()->stackModel()->setAutoUpdate(false);
    }
}

void KDevelop::FramestackWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    if (m_controller->currentSession()
        && m_controller->currentSession()->state() != KDevelop::IDebugSession::EndedState)
    {
        kDebug();
        StackModel* model = m_controller->currentSession()->stackModel();
        model->setAutoUpdate(true);
        mThreads->setModel(model);
        
        connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(assignSomeThread()));
        assignSomeThread();
        
        connect(m_controller->currentSession(), SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)),
                    SLOT(stateChanged(KDevelop::IDebugSession::DebuggerState)));
    }
}

void KDevelop::FramestackWidget::setThreadShown(const QModelIndex& idx)
{
    StackModel* model = m_controller->currentSession()->stackModel();
    FramesModel *f= model->modelForThread(idx.row());
    mFrames->setModel(f);
}

void KDevelop::FramestackWidget::checkFetchMoreFrames()
{
    int val=mFrames->verticalScrollBar()->value();
    int max=mFrames->verticalScrollBar()->maximum();
    const int offset=20;
    
    if(val+offset>max)
        static_cast<FramesModel *>(mFrames->model())->moreItems();
}

void KDevelop::FramestackWidget::assignSomeThread()
{
    StackModel* model=m_controller->currentSession()->stackModel();
    kDebug() << mFrames->model() << model->hasThreads();
    if(!mFrames->model() && model->hasThreads()) {
        QModelIndex idx=mThreads->model()->index(0,0);
        setThreadShown(idx);
    }
    if (false && model->rowCount() <= 1) {
        mThreadsWidget->hide();
    } else {
        mThreadsWidget->show();
    }
}

void FramestackWidget::stateChanged(KDevelop::IDebugSession::DebuggerState state)
{
    if (state == KDevelop::IDebugSession::EndedState) {
        mThreads->setModel(0);
        mFrames->setModel(0);
    }
}



#include "framestackwidget.moc"
