/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#include "problemwidget.h"

#include <QHeaderView>
#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include <klocale.h>
#include <kicon.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "duchain.h"
#include "duchainobserver.h"

#include "problemreporterplugin.h"
#include "problemmodel.h"

//#include "modeltest.h"

using namespace KDevelop;

ProblemWidget::ProblemWidget(QWidget* parent, ProblemReporterPlugin* plugin)
    : QTreeView(parent)
    , m_plugin(plugin)
{
    setObjectName("Problem Reporter Tree");
    setWindowTitle(i18n("Problem Reporter"));
    setWindowIcon(KIcon("info"));
    setRootIsDecorated(true);
    setWhatsThis( i18n( "Problem Reporter" ) );
    setModel(new ProblemModel(m_plugin));

    //new ModelTest(model());

    connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
    bool success = connect(DUChain::self()->notifier(), SIGNAL(problemEncountered(KDevelop::Problem)), SLOT(problemEncountered(KDevelop::Problem)), Qt::QueuedConnection);
    Q_ASSERT(success);
}

ProblemWidget::~ProblemWidget()
{
}

void ProblemWidget::problemEncountered(Problem problem)
{
    model()->addProblem(new Problem(problem));
}

void ProblemWidget::itemActivated(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    KDevelop::Problem* problem = model()->problemForIndex(index);

    if (!index.internalPointer())
        m_plugin->core()->documentController()->openDocument(KUrl(problem->finalLocation().document().str()), problem->finalLocation().start());
    else
        m_plugin->core()->documentController()->openDocument(KUrl(problem->locationStack().at(index.row()).document().str()), problem->locationStack().at(index.row()));
}

ProblemModel * ProblemWidget::model() const
{
    return static_cast<ProblemModel*>(QTreeView::model());
}

void ProblemWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

#include "problemwidget.moc"
