// -*- Mode: C++/l; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-

/* This file is part of KDevelop
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "widget.h"

#include <KIcon>
#include <KLocale>
#include <KDebug>

#include "plugin.h"
#include "job.h"
#include "imodel.h"
#include "cppcheckview.h"
#include "cppcheckmodel.h"
#include "cppcheck_file_model.h"
#include "cppcheck_severity_model.h"

#include <QResizeEvent>

namespace cppcheck
{

class ViewFactoryPrivate
{
public:
    static cppcheck::IView * make(cppcheck::Model * m);
};

cppcheck::IView * ViewFactoryPrivate::make(cppcheck::Model * m)
{
    if (dynamic_cast<cppcheck::CppcheckModel *>(m))
        return new cppcheck::CppcheckView();
    else if (dynamic_cast<cppcheck::CppcheckFileModel *>(m))
        return new cppcheck::CppcheckView();
    if (dynamic_cast<cppcheck::CppcheckSeverityModel *>(m))
        return new cppcheck::CppcheckView();
    kDebug() << "view not yet implemented";
    return 0;
}

Widget::Widget(cppcheck::Plugin * plugin, QWidget * parent)
    : QTabWidget(parent)
    , m_plugin(plugin)
{
    setWindowIcon(KIcon("fork"));
    setWindowTitle(i18n("Cppcheck Output"));

    setWhatsThis(i18n("<b>Cppcheck</b><p>Shows the output of cppcheck.</p>"));
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(destroyRequestedTab(int)));
    connect(plugin, SIGNAL(newModel(cppcheck::Model*)), this, SLOT(newModel(cppcheck::Model *)));
}

cppcheck::Plugin * Widget::plugin() const
{
    return m_plugin;
}

void Widget::newModel(cppcheck::Model * model)
{
    cppcheck::Job * job;

    job = model->job();
    if (job)
    {
        cppcheck::IView * w = ViewFactoryPrivate::make(model);

        w->setModel(model);
        connect(job, SIGNAL(updateTabText(cppcheck::Model *, const QString &)),
                this, SLOT(updateTabText(cppcheck::Model *, const QString &)));
        addTab(dynamic_cast<QWidget *>(w), i18n("Job scheduled"));
        setCurrentWidget(dynamic_cast<QWidget *>(w));
        setMovable(true);
    }
}

void Widget::destroyRequestedTab(int index)
{
    cppcheck::IView * view = dynamic_cast<cppcheck::IView *>(widget(index));

    // kill the job if it's still running
    if (view)
    {
        cppcheck::Model * model = dynamic_cast<cppcheck::IView *>(widget(index))->model();
        if (model)
        {
            if (model->job())
            {
                model->job()->doKill();
            }
            delete model;
        }
    }
    removeTab(index);
}

void Widget::updateTabText(cppcheck::Model * model, const QString & text)
{
    for (int i = 0; i < count(); ++i)
    {
        cppcheck::IView * view = dynamic_cast<cppcheck::IView *>(widget(i));
        if (view && view->model() == model)
        {
            setTabText(i, text);
        }
    }
}

void Widget::resizeEvent( QResizeEvent *event )
{
    for (int i = 0; i < count(); ++i)
    {
        //notify child size has changed
        IView *view = dynamic_cast<IView *>(widget(i));
        if (view != 0)
            view->WidgetContainerResizeEvent(event);
    }
}

}

#include "widget.moc"
