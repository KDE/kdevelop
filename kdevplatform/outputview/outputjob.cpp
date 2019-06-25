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
#include <QIcon>
#include <QPointer>

#include "interfaces/icore.h"
#include "interfaces/iplugincontroller.h"
#include "outputview/ioutputview.h"

using namespace KDevelop;

class KDevelop::OutputJobPrivate
{
public:
    explicit OutputJobPrivate(OutputJob::OutputJobVerbosity verbosity) : verbosity(verbosity) {}

    int standardToolView = -1;
    QString title;
    QString toolTitle;
    QIcon toolIcon;
    IOutputView::ViewType type = IOutputView::OneView;
    IOutputView::Behaviours behaviours = IOutputView::AllowUserClose;
    bool killJobOnOutputClose = true;
    OutputJob::OutputJobVerbosity verbosity;
    int outputId = -1;
    QPointer<QAbstractItemModel> outputModel;
    QAbstractItemDelegate* outputDelegate = nullptr;
};

OutputJob::OutputJob(QObject* parent, OutputJobVerbosity verbosity)
    : KJob(parent)
    , d_ptr(new OutputJobPrivate(verbosity))
{
}

OutputJob::~OutputJob() = default;

void OutputJob::startOutput()
{
    Q_D(OutputJob);

    IPlugin* i = ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IOutputView"));
    if( i )
    {
        auto* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            int tvid;
            if (d->standardToolView != -1) {
                tvid = view->standardToolView(static_cast<IOutputView::StandardToolView>(d->standardToolView));
            } else {
                tvid = view->registerToolView(d->toolTitle, d->type, d->toolIcon);
            }

            if (d->title.isEmpty())
                d->title = objectName();

            d->outputId = view->registerOutputInToolView(tvid, d->title, d->behaviours);

            if (!d->outputModel) {
                d->outputModel = new QStandardItemModel(nullptr);
            }

            // Keep the item model around after the job is gone
            view->setModel(d->outputId, d->outputModel);

            if (!d->outputDelegate) {
                d->outputDelegate = new QItemDelegate(nullptr);
            }

            view->setDelegate(d->outputId, d->outputDelegate);

            if (d->killJobOnOutputClose) {
                // can't use qt5 signal slot syntax here, IOutputView is no a QObject
                connect(i, SIGNAL(outputRemoved(int,int)), this, SLOT(outputViewRemoved(int,int)));
            }

            if (d->verbosity == OutputJob::Verbose)
                view->raiseOutput(d->outputId);
        }
    }
}

void OutputJob::outputViewRemoved(int toolViewId, int id)
{
    Q_D(OutputJob);

    Q_UNUSED(toolViewId);
    if (id == d->outputId && d->killJobOnOutputClose) {
        // Make sure that the job emits result signal as the job
        // might be used in composite jobs and that one depends
        // on result being emitted to know whether a subjob
        // is done.
        kill( KJob::EmitResult );
    }
}

void KDevelop::OutputJob::setTitle(const QString & title)
{
    Q_D(OutputJob);

    d->title = title;
    if (d->outputId >= 0 && d->standardToolView >= 0) {
        IPlugin* i = ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IOutputView"));
        if( i )
        {
            auto* view = i->extension<KDevelop::IOutputView>();
            if( view )
            {
                view->setTitle(d->outputId, title);
            }
        }
    }
}

void KDevelop::OutputJob::setViewType(IOutputView::ViewType type)
{
    Q_D(OutputJob);

    d->type = type;
}

void KDevelop::OutputJob::setBehaviours(IOutputView::Behaviours behaviours)
{
    Q_D(OutputJob);

    d->behaviours = behaviours;
}

void KDevelop::OutputJob::setKillJobOnOutputClose(bool killJobOnOutputClose)
{
    Q_D(OutputJob);

    d->killJobOnOutputClose = killJobOnOutputClose;
}

void KDevelop::OutputJob::setModel(QAbstractItemModel * model)
{
    Q_D(OutputJob);

    if (d->outputModel) {
        delete d->outputModel;
    }

    d->outputModel = model;

    if (d->outputModel) {
        d->outputModel->setParent(this);
    }
}

void KDevelop::OutputJob::setDelegate(QAbstractItemDelegate * delegate)
{
    Q_D(OutputJob);

    d->outputDelegate = delegate;
}

QAbstractItemModel * KDevelop::OutputJob::model() const
{
    Q_D(const OutputJob);

    return d->outputModel;
}

void KDevelop::OutputJob::setStandardToolView(IOutputView::StandardToolView standard)
{
    Q_D(OutputJob);

    d->standardToolView = standard;
}

void OutputJob::setToolTitle(const QString& title)
{
    Q_D(OutputJob);

    d->toolTitle = title;
}

void OutputJob::setToolIcon(const QIcon& icon)
{
    Q_D(OutputJob);

    d->toolIcon = icon;
}

int OutputJob::outputId() const
{
    Q_D(const OutputJob);

    return d->outputId;
}

OutputJob::OutputJobVerbosity OutputJob::verbosity() const
{
    Q_D(const OutputJob);

    return d->verbosity;
}

void OutputJob::setVerbosity(OutputJob::OutputJobVerbosity verbosity)
{
    Q_D(OutputJob);

    d->verbosity = verbosity;
}

