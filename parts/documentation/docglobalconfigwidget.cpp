/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *   Copyright (C) 2003-2004 by Alexander Dymo                             *
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

#include <qlayout.h>
#include <qtoolbox.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <kapplication.h>
#include <kurlrequester.h>

#include "kdevdocumentationplugin.h"

#include "docconfiglistview.h"
#include "documentation_part.h"
#include "documentation_widget.h"
#include "editcatalogdlg.h"
#include "contentsview.h"
#include "indexview.h"
#include "docutils.h"

DocGlobalConfigWidget::DocGlobalConfigWidget(DocumentationPart *part, 
    DocumentationWidget *widget, QWidget *parent, const char *name, WFlags fl)
    :DocGlobalConfigWidgetBase(parent, name, fl), m_part(part), m_widget(widget)
{
    //load catalog settings
    if (collectionsBox->currentItem())
        collectionsBox->removeItem(collectionsBox->currentItem());
    for (QValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
        it != m_part->m_plugins.constEnd(); ++it)
    {
        QWidget *contentsContainter = new QWidget(this);
        QVBoxLayout *cl = new QVBoxLayout(contentsContainter, 0, 0);
        DocConfigListView *view = new DocConfigListView(contentsContainter);
        cl->addWidget(view);
        int box = collectionsBox->addItem(contentsContainter, (*it)->pluginName());
        m_pluginBoxes[box] = *it;
        m_pluginViews[box] = view;
        (*it)->loadCatalogConfiguration(m_pluginViews[box]);
    }
    
    KConfig *config = m_part->config();
    //read full text search settings
    config->setGroup("htdig");
    QString databaseDir = kapp->dirs()->saveLocation("data",
        "kdevdocumentation/search");
    databaseDirEdit->setURL(config->readPathEntry("databaseDir", databaseDir));
    htdigbinEdit->setURL(config->readPathEntry("htdigbin", kapp->dirs()->findExe("htdig")));
    htmergebinEdit->setURL(config->readPathEntry("htmergebin", kapp->dirs()->findExe("htmerge")));
    htsearchbinEdit->setURL(config->readPathEntry("htsearchbin", kapp->dirs()->findExe("htsearch")));
    if (config->readBoolEntry("IsSetup", false) == false)
    {
        QFileInfo fi(htsearchbinEdit->url());
        if (!fi.exists())
        {
            //try to guess htsearch location on Debian
            QFileInfo fi("/usr/lib/cgi-bin/htsearch");
            if (!fi.exists())
            {
                //try to guess htsearch location on SuSE
                QFileInfo fi("/srv/www/cgi-bin/htsearch");
                if (fi.exists())
                    htsearchbinEdit->setURL("/srv/www/cgi-bin/htsearch");
            }
            else
                htsearchbinEdit->setURL("/usr/lib/cgi-bin/htsearch");
        }
    }
    
    find_box->setChecked(m_part->hasContextFeature(DocumentationPart::Finder));
    index_box->setChecked(m_part->hasContextFeature(DocumentationPart::IndexLookup));
    search_box->setChecked(m_part->hasContextFeature(DocumentationPart::FullTextSearch));
    man_box->setChecked(m_part->hasContextFeature(DocumentationPart::GotoMan));
    info_box->setChecked(m_part->hasContextFeature(DocumentationPart::GotoInfo));
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
    dlg.setURL(item->url());
    dlg.setTitle(item->title());
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

void DocGlobalConfigWidget::accept()
{
    //write catalog settings
    for (QMap<int, DocumentationPlugin*>::const_iterator it = m_pluginBoxes.constBegin();
        it != m_pluginBoxes.constEnd(); ++ it)
    {
        it.data()->saveCatalogConfiguration(m_pluginViews[it.key()]);
        //@todo: take restrictions into account
        it.data()->reinit(m_widget->contents(), m_widget->index(), QStringList());
    }

    KConfig *config = m_part->config();
    //write full text search settings
    config->setGroup("htdig");
    config->writePathEntry("databaseDir", DocUtils::envURL(databaseDirEdit));
    config->writePathEntry("htdigbin", DocUtils::envURL(htdigbinEdit));
    config->writePathEntry("htmergebin", DocUtils::envURL(htmergebinEdit));
    config->writePathEntry("htsearchbin", DocUtils::envURL(htsearchbinEdit));
    
    //write full text search locations file    
    QString ftsLocationsFile = locateLocal("data", "kdevdocumentation/search/locations.txt");
    QFile f(ftsLocationsFile);
    QStringList locs;
    if (f.open(IO_ReadWrite | IO_Truncate))
    {
        QTextStream str(&f);
        for (QValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
            it != m_part->m_plugins.constEnd(); ++ it)
        {
            QStringList app = (*it)->fullTextSearchLocations();
            for (QStringList::const_iterator it2 = app.constBegin();
                it2 != app.constEnd(); ++it2)
            {
                if (!locs.contains(*it2))
                    locs.append(*it2);
            }
        }
        str << locs.join("\n");

        f.close();
    }
    
    //write editor context menu configuration
    m_part->setContextFeature(DocumentationPart::Finder, find_box->isChecked());
    m_part->setContextFeature(DocumentationPart::IndexLookup, index_box->isChecked());
    m_part->setContextFeature(DocumentationPart::FullTextSearch, search_box->isChecked());
    m_part->setContextFeature(DocumentationPart::GotoMan, man_box->isChecked());
    m_part->setContextFeature(DocumentationPart::GotoInfo, info_box->isChecked());
    
    config->sync();
    
    //refill the index
    kdDebug() << "refill the index" << endl;    
    if (m_part->m_hasIndex)
    {
        kdDebug() << "m_part->m_hasIndex" << endl;
        m_part->m_widget->index()->refill();
    }
}

#include "docglobalconfigwidget.moc"
