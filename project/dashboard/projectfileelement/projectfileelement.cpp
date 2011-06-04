/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "projectfileelement.h"

#include <analitza/expression.h>
#include <plasma/context.h>
#include <plasma/datacontainer.h>
#include <plasma/dataenginemanager.h>
#include <QFile>
#include <QFormLayout>
#include <KConfigDialog>

using namespace Plasma;

ProjectFileItem::ProjectFileItem(QObject *parent, const QVariantList &args)
	: Applet(parent, args), m_args(args)
{
	setAspectRatioMode(IgnoreAspectRatio);
}

ProjectFileItem::~ProjectFileItem() {}

void ProjectFileItem::init()
{
    if(!m_args.isEmpty())
        config().writeEntry("relativePath", m_args.first());
    
    QGraphicsLinearLayout* m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_output = new Plasma::Label(this);
    m_layout->addItem(m_output);
    
    setLayout(m_layout);
    setPreferredSize(300,300);
    
    reloadData();
}

void ProjectFileItem::reloadData()
{
    DataEngine* thedata = dataEngine("org.kdevelop.projects");
    
    DataContainer* projectUrl=thedata->containerForSource(context()->currentActivity());
    
    KUrl url=projectUrl->data().value("projectFileUrl").value<KUrl>();
    url=url.upUrl();
    url.addPath(config().readEntry("relativePath"));
    
    QFile f(url.toLocalFile());
    if(f.open(QFile::ReadOnly)) {
        m_output->setText(f.readAll());
    }
}

void ProjectFileItem::createConfigurationInterface(KConfigDialog* parent)
{
    QWidget* w=new QWidget(parent);
    m_ui.setupUi(w);
    m_ui.relativePath->setText(config().readEntry("relativePath"));
    
    parent->addPage(w, i18n("File"));
    connect(parent, SIGNAL(applyClicked()), SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), SLOT(configAccepted()));
}

void ProjectFileItem::configAccepted()
{
    config().writeEntry("relativePath", m_ui.relativePath->text());
    
    reloadData();
}

#include "projectfileelement.moc"
