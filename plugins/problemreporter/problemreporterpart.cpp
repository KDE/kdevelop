/*
 * KDevelop Problem Reporter
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "problemreporterpart.h"

#include <QTreeWidget>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>

#include "problemwidget.h"

K_PLUGIN_FACTORY(KDevProblemReporterFactory, registerPlugin<ProblemReporterPart>(); )
K_EXPORT_PLUGIN(KDevProblemReporterFactory("kdevproblemreporter"))

using namespace KDevelop;

class ProblemReporterFactory: public KDevelop::IToolViewFactory
{
public:
  ProblemReporterFactory(ProblemReporterPart *part): m_part(part) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new ProblemWidget(parent, m_part);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  ProblemReporterPart *m_part;
};

ProblemReporterPart::ProblemReporterPart(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevProblemReporterFactory::componentData(), parent)
    , m_factory(new ProblemReporterFactory(this))
{
  core()->uiController()->addToolView(i18n("Problem Reporter"), m_factory);
  setXMLFile( "kdevproblemreporter.rc" );
}

ProblemReporterPart::~ProblemReporterPart()
{
}

void ProblemReporterPart::unload()
{
  core()->uiController()->removeToolView(m_factory);
}

#include "problemreporterpart.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
