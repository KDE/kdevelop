/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "docprojectconfigwidget.h"

#include <qdir.h>
#include <qcombobox.h>

#include <kdebug.h>
#include <kurlrequester.h>

#include "domutil.h"
#include "urlutil.h"
#include "kdevproject.h"
#include "kdevdocumentationplugin.h"

#include "documentation_part.h"
#include "documentation_widget.h"

DocProjectConfigWidget::DocProjectConfigWidget(DocumentationPart *part, QWidget *parent, const char *name)
    :DocProjectConfigWidgetBase(parent, name), m_part(part)
{
    for (QValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
        it != m_part->m_plugins.constEnd(); ++it)
    {
        if ((*it)->hasCapability(DocumentationPlugin::ProjectDocumentation))
        {
            docSystemCombo->insertItem((*it)->pluginName());
            m_plugins[(*it)->pluginName()] = *it;
        }
    }
    QString projectDocSystem = DomUtil::readEntry(*(m_part->projectDom()), "/kdevdocumentation/projectdoc/docsystem");
    
    bool hasProjectDoc = false;
    for (int i = 0; i < docSystemCombo->count(); ++i)
    {
        if (docSystemCombo->text(i) == projectDocSystem)
        {
            docSystemCombo->setCurrentItem(i);
	    hasProjectDoc = true;
	    changeDocSystem(docSystemCombo->currentText());
            break;
        }
    }
    if (!hasProjectDoc && docSystemCombo->count() > 0)
    {
        docSystemCombo->setCurrentItem(0);
        changeDocSystem(docSystemCombo->currentText());
    }
}

void DocProjectConfigWidget::changeDocSystem(const QString &text)
{
    if (text.isEmpty())
        return;
    
    DocumentationPlugin *plugin = m_plugins[text];
    if (!plugin)
        return;
    
    catalogURL->setMode(plugin->catalogLocatorProps().first);
    catalogURL->setFilter(plugin->catalogLocatorProps().second);
    
    QString projectDocURL = DomUtil::readEntry(*(m_part->projectDom()), "/kdevdocumentation/projectdoc/docurl");
    if (!projectDocURL.isEmpty())
        projectDocURL = QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + projectDocURL);

    catalogURL->setURL(projectDocURL);
    catalogURL->setEnabled(true);
}

void DocProjectConfigWidget::accept()
{
    if (docSystemCombo->currentText().isEmpty())
        return;
    if (catalogURL->url().isEmpty())
    {
        if (m_part->m_projectDocumentationPlugin)
        {
            delete m_part->m_projectDocumentationPlugin;
            m_part->m_projectDocumentationPlugin = 0;
        }
        m_part->saveProjectDocumentationInfo();
        return;
    }
    
    DocumentationPlugin *plugin = m_plugins[docSystemCombo->currentText()];
    if (!plugin)
        return;
    
    if (m_part->m_projectDocumentationPlugin)
    {
        delete m_part->m_projectDocumentationPlugin;
        m_part->m_projectDocumentationPlugin = 0;
    }
    m_part->m_projectDocumentationPlugin = plugin->projectDocumentationPlugin();
    m_part->m_projectDocumentationPlugin->init(m_part->m_widget->contents(), m_part->m_widget->index(), catalogURL->url());
    m_part->saveProjectDocumentationInfo();
}

#include "docprojectconfigwidget.moc"
