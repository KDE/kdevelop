/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include "docglobalconfigwidget.h"

#include <qtoolbox.h>

#include "kdevdocumentationplugin.h"

#include "docconfiglistview.h"
#include "documentation_part.h"
#include "documentation_widget.h"
#include "editcatalogdlg.h"
#include "contentsview.h"
#include "indexview.h"

DocGlobalConfigWidget::DocGlobalConfigWidget(DocumentationPart *part, 
    DocumentationWidget *widget, QWidget *parent, const char *name, WFlags fl)
    :DocGlobalConfigWidgetBase(parent, name, fl), m_part(part), m_widget(widget)
{
    if (collectionsBox->currentItem())
        collectionsBox->removeItem(collectionsBox->currentItem());
    for (QValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
        it != m_part->m_plugins.constEnd(); ++it)
    {
        DocConfigListView *view = new DocConfigListView(this);
        int box = collectionsBox->addItem(view, (*it)->pluginName());
        m_pluginBoxes[box] = *it;
        m_pluginViews[box] = view;
        (*it)->loadCatalogConfiguration(m_pluginViews[box]);
    }
}

DocGlobalConfigWidget::~DocGlobalConfigWidget()
{
}

/*$SPECIALIZATION$*/
void DocGlobalConfigWidget::collectionsBoxCurrentChanged(int box)
{
}

void DocGlobalConfigWidget::removeCollectionButtonClicked()
{
    ConfigurationItem *item = dynamic_cast<ConfigurationItem*>(activeView()->currentItem());
    if (!item)
        return;
    activePlugin()->deleteCatalogConfiguration(item);
    delete activeView()->currentItem();
}

void DocGlobalConfigWidget::editCollectionButtonClicked()
{
    ConfigurationItem *item = dynamic_cast<ConfigurationItem*>(activeView()->currentItem());
    if (!item)
        return;
    EditCatalogDlg dlg(activePlugin(), this, "edit collection dlg", true);
    dlg.setTitle(item->title());
    dlg.setURL(item->url());
    if (dlg.exec())
        activePlugin()->editCatalogConfiguration(item, dlg.title(), dlg.url());
}

void DocGlobalConfigWidget::addCollectionButtonClicked()
{
    EditCatalogDlg dlg(activePlugin(), this, "edit collection dlg", true);
    if (dlg.exec())
        activePlugin()->addCatalogConfiguration(activeView(), dlg.title(), dlg.url());
}

DocumentationPlugin *DocGlobalConfigWidget::activePlugin()
{
    return m_pluginBoxes[collectionsBox->currentIndex()];
}

KListView *DocGlobalConfigWidget::activeView()
{
    return m_pluginViews[collectionsBox->currentIndex()];
}

void DocGlobalConfigWidget::accept( )
{
    for (QMap<int, DocumentationPlugin*>::const_iterator it = m_pluginBoxes.constBegin();
        it != m_pluginBoxes.constEnd(); ++ it)
    {
        it.data()->saveCatalogConfiguration(m_pluginViews[it.key()]);
        //@todo: take restrictions into account
        it.data()->reinit(m_widget->contents(), m_widget->index(), QStringList());
    }
}

#include "docglobalconfigwidget.moc"
