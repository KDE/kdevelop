/*
 * Copyright 2015 Laszlo Kis-Adam
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

#include "problemsview.h"
#include <QToolBar>
#include <QTabWidget>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <shell/problemmodelset.h>
#include "problemtreeview.h"
#include "problemmodel.h"

namespace KDevelop
{

ProblemsView::ProblemsView(QWidget *parent) :
QMainWindow(parent)
{
    m_toolBar = new QToolBar(this);
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
    addToolBar(m_toolBar);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::South);
    setCentralWidget(m_tabWidget);
}

ProblemsView::~ProblemsView()
{
}


void ProblemsView::load()
{
    m_tabWidget->clear();

    KDevelop::ProblemModelSet *pms = KDevelop::ICore::self()->languageController()->problemModelSet();
    QVector<KDevelop::ModelData> v = pms->models();

    QVectorIterator<KDevelop::ModelData> itr(v);
    while (itr.hasNext()) {
        const KDevelop::ModelData &data = itr.next();
        addModel(data);
    }

    connect(pms, &ProblemModelSet::added, this, &ProblemsView::onModelAdded);
    connect(pms, &ProblemModelSet::removed, this, &ProblemsView::onModelRemoved);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &ProblemsView::onCurrentChanged);

    if ( m_tabWidget->currentIndex() == 0 ) {
        updateToolBar();
        return;
    }

    m_tabWidget->setCurrentIndex(0);
}

void ProblemsView::onModelAdded(const ModelData &data)
{
    addModel(data);
}

// Returns the name part of the label
// E.g.: Test(666) => Test
QString nameFromLabel(const QString &label)
{
    QString txt = label;
    int i = txt.lastIndexOf('(');
    if (i != -1)
        txt = txt.left(i);
    return txt;
}

void ProblemsView::onModelRemoved(const QString &name)
{
    int c = m_tabWidget->count();
    int idx = 0;
    for (idx = 0; idx < c; ++idx) {
        if(nameFromLabel(m_tabWidget->tabText(idx)) == name)
            break;
    }

    if (idx < c) {
        QWidget *w = m_tabWidget->widget(idx);
        m_tabWidget->removeTab(idx);
        delete w;
    }
}

void ProblemsView::onCurrentChanged(int idx)
{
    m_toolBar->clear();

    if (idx == -1)
        return;

    updateToolBar();
}

void ProblemsView::onViewChanged()
{
    ProblemTreeView *view = static_cast<ProblemTreeView*>(sender());
    int idx = m_tabWidget->indexOf(view);
    int rows = view->model()->rowCount();

    updateTab(idx, rows);
}


void ProblemsView::addModel(const ModelData &data)
{
    ProblemTreeView *view = new ProblemTreeView(NULL,data.model);
    connect(view, &ProblemTreeView::changed, this, &ProblemsView::onViewChanged);
    int idx = m_tabWidget->addTab(view, data.name);
    int rows = view->model()->rowCount();

    updateTab(idx, rows);
}

void ProblemsView::updateToolBar()
{
    QWidget *w = m_tabWidget->currentWidget();
    m_toolBar->addActions(w->actions());
}

void ProblemsView::updateTab(int idx, int rows)
{
    const QString txt = nameFromLabel(m_tabWidget->tabText(idx)) + '(' + QString::number(rows) + ')';
    m_tabWidget->setTabText(idx, txt);
}

}


