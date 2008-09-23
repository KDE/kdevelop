/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "itestframework.h"
#include "toolviewdata.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/document.h>
#include <sublime/view.h>

using KDevelop::ICore;
using KDevelop::IUiController;
using Sublime::Area;
using Sublime::Document;
using Sublime::View;
using Veritas::ITestFramework;
using Veritas::ITestFrameworkPrivate;
using Veritas::ToolViewData;

namespace
{

/*! Functor that looks for a results-view in a Sublime::Area 
 *  If one is found, @p found becomes true */
class ResultsViewFinder
{
public:
    ResultsViewFinder(const QString& id) : m_id(id), found(false) {}
    Area::WalkerMode operator()(View *view, Sublime::Position position) {
        Document* doc = view->document();
        if (doc->documentSpecifier().startsWith(m_id)) {
            found = true;
            m_view = view;
            return Area::StopWalker;
        } else {
            return Area::ContinueWalker;
        }
    }
    QString m_id;
    bool found;
    View* m_view;
};

}

class ITestFrameworkPrivate : public QObject
{
Q_OBJECT
public:
    ITestFrameworkPrivate() {
        Sublime::Controller* c = ICore::self()->uiController()->controller();
        connect(c, SIGNAL(aboutToRemoveToolView(Sublime::View*)), SLOT(maybeRemoveResultsView(Sublime::View*)));
        connect(c, SIGNAL(toolViewMoved(Sublime::View*)), SLOT(fixMovedResultsView(Sublime::View*)));
    }
    ITestFramework* self;

private:
    void removeResultsView(const QString& docId) {
        IUiController* uic = ICore::self()->uiController();
        Sublime::Controller* sc = uic->controller();
        sc->disconnect(this);
        QList<Area*> as = sc->allAreas();
        foreach(Area* a, as) {
            ResultsViewFinder rvf(docId);
            a->walkToolViews(rvf, Sublime::AllPositions);
            if (rvf.found) {
                a->removeToolView(rvf.m_view);
            }
        }
        connect(sc, SIGNAL(aboutToRemoveToolView(Sublime::View*)), SLOT(maybeRemoveResultsView(Sublime::View*)));
        connect(sc, SIGNAL(toolViewMoved(Sublime::View*)), SLOT(fixMovedResultsView(Sublime::View*)));
    }

private slots:
    void fixMovedResultsView(Sublime::View* v) {
        maybeRemoveResultsView(v);
        Q_ASSERT(g_toolViewStore.contains(self));
        ToolViewData& tv = g_toolViewStore[self];
        if (tv.view2id.contains(v)) {
            tv.view2id[v] = tv.runnerToolCounter;
        }
    }

    void maybeRemoveResultsView(Sublime::View* v) {
        Q_ASSERT(g_toolViewStore.contains(self));
        ToolViewData& tv = g_toolViewStore[self];
        if (tv.view2id.contains(v)) {
            QString docId = QString("org.kdevelop.%1ResultsView").arg(self->name());
            docId += QString::number(tv.view2id[v]);
            removeResultsView(docId);
        }
    }

};

ITestFramework::ITestFramework()
    : d(new ITestFrameworkPrivate())
{
    d->self = this;
    g_toolViewStore[this] = ToolViewData();
}

ITestFramework::~ITestFramework()
{
    delete d;
}

#include "moc_itestframework.cpp"
#include "itestframework.moc"
