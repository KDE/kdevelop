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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "docprojectconfigwidget.h"

#include <qdir.h>
#include <qcombobox.h>
//Added by qt3to4:
#include <Q3ValueList>

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
    for (Q3ValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
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

    manualURL->setURL(DomUtil::readEntry(*(m_part->projectDom()), "/kdevdocumentation/projectdoc/usermanualurl"));
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
    if (manualURL->url().isEmpty())
    {
        if (m_part->m_userManualPlugin)
        {
            delete m_part->m_userManualPlugin;
            m_part->m_userManualPlugin = 0;
        }
    }
    else
    {
        if (m_part->m_userManualPlugin)
        {
            delete m_part->m_userManualPlugin;
            m_part->m_userManualPlugin = 0;
        }
        for (Q3ValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
            it != m_part->m_plugins.constEnd(); ++it)
        {
            if ((*it)->hasCapability(DocumentationPlugin::ProjectUserManual))
                m_part->m_userManualPlugin = (*it)->projectDocumentationPlugin(DocumentationPlugin::UserManual);
        }
        if (m_part->m_userManualPlugin)
            m_part->m_userManualPlugin->init(m_part->m_widget->contents(), m_part->m_widget->index(), manualURL->url());
    }
    m_part->saveProjectDocumentationInfo();
    
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
    m_part->m_projectDocumentationPlugin = plugin->projectDocumentationPlugin(DocumentationPlugin::APIDocs);
    m_part->m_projectDocumentationPlugin->init(m_part->m_widget->contents(), m_part->m_widget->index(), catalogURL->url());
    
    m_part->saveProjectDocumentationInfo();
}

#include "docprojectconfigwidget.moc"
