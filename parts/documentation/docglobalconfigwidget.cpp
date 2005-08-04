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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "docglobalconfigwidget.h"

#include <qlayout.h>
#include <qtoolbox.h>
#include <qcheckbox.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QTextStream>

#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kfontcombo.h>
#include <khtml_part.h>
#include <khtml_settings.h>

#include "kdevdocumentationplugin.h"
#include "kdevpartcontroller.h"

#include "docconfiglistview.h"
#include "documentation_part.h"
#include "documentation_widget.h"
#include "editcatalogdlg.h"
#include "addcatalogdlg.h"
#include "contentsview.h"
#include "indexview.h"
#include "docutils.h"

DocGlobalConfigWidget::DocGlobalConfigWidget(DocumentationPart *part,
    DocumentationWidget *widget, QWidget *parent, const char *name, Qt::WFlags fl)
    :DocGlobalConfigWidgetBase(parent, name, fl), m_part(part), m_widget(widget)
{
    m_View = new DocConfigListView( viewHolder );
    viewHolder->addWidget( m_View );
    viewHolder->raiseWidget( m_View );
    for (Q3ValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
        it != m_part->m_plugins.constEnd(); ++it)
    {
        (*it)->loadCatalogConfiguration( m_View );
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

    useAssistant_box->setChecked(m_part->isAssistantUsed());

    // Having app-specific settings isn't pretty, but this setting is nonsensical in kdevassistant
    if ( kapp->instanceName().find("kdevassistant") != -1 )
        useAssistant_box->hide();

    //font sizes and zoom levels
    KHTMLPart htmlpart;
    KConfig *appConfig = KGlobal::config();
    appConfig->setGroup("KHTMLPart");
    standardFont_combo->setCurrentText(appConfig->readEntry("StandardFont",
        htmlpart.settings()->stdFontName()));
    fixedFont_combo->setCurrentText(appConfig->readEntry("FixedFont",
        htmlpart.settings()->fixedFontName()));
    zoom_combo->setCurrentText(appConfig->readEntry("Zoom", "100"));
}

DocGlobalConfigWidget::~DocGlobalConfigWidget()
{
}

/*$SPECIALIZATION$*/
void DocGlobalConfigWidget::collectionsBoxCurrentChanged(int // box
                                                         )
{
}

void DocGlobalConfigWidget::removeCollectionButtonClicked()
{
    ConfigurationItem *item = dynamic_cast<ConfigurationItem*>(activeView()->currentItem());
    if (!item)
        return;
    item->docPlugin()->deleteCatalogConfiguration(item);
    delete activeView()->currentItem();
}

void DocGlobalConfigWidget::editCollectionButtonClicked()
{
    ConfigurationItem *item = dynamic_cast<ConfigurationItem*>(activeView()->currentItem());
    if (!item)
        return;
    EditCatalogDlg dlg( item->docPlugin(), this, "edit collection dlg", true);
    dlg.setURL(item->url());
    dlg.setTitle(item->title());
    if (dlg.exec())
        item->docPlugin()->editCatalogConfiguration(item, dlg.title(), dlg.url());
}

void DocGlobalConfigWidget::addCollectionButtonClicked()
{
	AddCatalogDlg dlg( m_part->m_plugins, this, "add collection dlg", true);
	if (dlg.exec())
	{
		dlg.plugin()->addCatalogConfiguration(activeView(), dlg.title(), dlg.url());
	}
}

KListView *DocGlobalConfigWidget::activeView()
{
    return m_View;
}

void DocGlobalConfigWidget::accept()
{
    //write catalog settings
	for (Q3ValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin(); 
		it != m_part->m_plugins.constEnd(); ++it)
	{
		(*it)->saveCatalogConfiguration( m_View );
		//@todo: take restrictions into account
		(*it)->reinit(m_widget->contents(), m_widget->index(), QStringList());
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
    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        QTextStream str(&f);
        for (Q3ValueList<DocumentationPlugin*>::const_iterator it = m_part->m_plugins.constBegin();
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

    m_part->setAssistantUsed(useAssistant_box->isChecked());

    //font sizes and zoom levels
    KConfig *appConfig = KGlobal::config();
    appConfig->setGroup("KHTMLPart");
    appConfig->writeEntry("StandardFont", standardFont_combo->currentText());
    appConfig->writeEntry("FixedFont", fixedFont_combo->currentText());
    appConfig->writeEntry("Zoom", zoom_combo->currentText());

    appConfig->sync();
    updateConfigForHTMLParts();
    config->sync();

    //refill the index
    kdDebug() << "refill the index" << endl;
    if (m_part->m_hasIndex)
    {
        kdDebug() << "m_part->m_hasIndex" << endl;
        m_part->m_widget->index()->refill();
    }
}

void DocGlobalConfigWidget::updateConfigForHTMLParts()
{
    KURL::List urls = m_part->partController()->openURLs();
    for (KURL::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        KHTMLPart *htmlPart = dynamic_cast<KHTMLPart*>(m_part->partController()->partForURL(*it));
        if (htmlPart)
        {
            KConfig *appConfig = KGlobal::config();
            appConfig->setGroup("KHTMLPart");
            htmlPart->setStandardFont(appConfig->readEntry("StandardFont"));
            htmlPart->setFixedFont(appConfig->readEntry("FixedFont"));
            //hack to force reloading the page
            if (htmlPart->zoomFactor() == appConfig->readEntry("Zoom").toInt())
            {
                htmlPart->setZoomFactor(htmlPart->zoomFactor()-1);
                htmlPart->setZoomFactor(htmlPart->zoomFactor()+1);
            }
            htmlPart->setZoomFactor(appConfig->readEntry("Zoom").toInt());
        }
    }
}

#include "docglobalconfigwidget.moc"
