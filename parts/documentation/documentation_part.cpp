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
#include "documentation_part.h"

#include <qwhatsthis.h>
#include <qlayout.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kservice.h>
#include <kdialogbase.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevgenericfactory.h"
#include "kdevdocumentationplugin.h"
#include "configwidgetproxy.h"

#include "documentation_widget.h"
#include "docglobalconfigwidget.h"
#include "contentsview.h"

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

static const KAboutData data("kdevdocumentation", I18N_NOOP("Documentation"), "1.0");

typedef KDevGenericFactory<DocumentationPart> DocumentationFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdocumentation, DocumentationFactory( &data ) );

DocumentationPart::DocumentationPart(QObject *parent, const char *name, const QStringList& )
    :KDevPlugin("Documentation", "doctree", parent, name ? name : "DocumentationPart" )
{
    setInstance(DocumentationFactory::instance());
    setXMLFile("kdevpart_documentation.rc");

    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("Documentation"), GLOBALDOC_OPTIONS);
/*    m_configProxy->createProjectConfigPage(i18n("Project Documentation"), PROJECTDOC_OPTIONS);*/
    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )), this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));
    
    m_widget = new DocumentationWidget(this);

    QWhatsThis::add(m_widget, i18n("<b>Documentation browser</b><p>"
        "The documentation browser gives access to various "
        "documentation sources (Qt DCF, Doxygen, KDoc, KDevelopTOC and DevHelp "
        "documentation) and the KDevelop manuals. It also provides documentation index "
        "and full text search capabilities."));

    mainWindow()->embedSelectViewRight(m_widget, i18n("Documentation"),
        i18n("Documentation browser"));
    
    loadDocumentationPlugins();
}

DocumentationPart::~DocumentationPart()
{
    if ( m_widget )
    {
        mainWindow()->removeView( m_widget );
    }
    delete m_widget;
}

void DocumentationPart::loadDocumentationPlugins()
{
    KTrader::OfferList docPluginOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/DocumentationPlugins"));

    KTrader::OfferList::ConstIterator serviceIt = docPluginOffers.begin();
    for ( ; serviceIt != docPluginOffers.end(); ++serviceIt )
    {               
        KService::Ptr docPluginService;
        docPluginService = *serviceIt;
        kdDebug() << "DocumentationPart::loadDocumentationPlugins: creating plugin" 
            << docPluginService->name() << endl;
        
        int error;
        DocumentationPlugin *docPlugin = KParts::ComponentFactory
            ::createInstanceFromService<DocumentationPlugin>(docPluginService, 0,
            docPluginService->name().latin1(), QStringList(), &error);
        if (!docPlugin)
            kdDebug() << "    failed to create doc plugin " << docPluginService->name() << endl;
        else
        {
            kdDebug() << "    success" << endl;
            docPlugin->init(m_widget->contents());
            connect(this, SIGNAL(indexSelected(KListBox* )), docPlugin, SLOT(createIndex(KListBox* )));
            m_plugins.append(docPlugin);
        }
    }
}

void DocumentationPart::emitIndexSelected(KListBox *indexBox)
{
    emit indexSelected(indexBox);
}

void DocumentationPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
    switch (pageNo)
    {
        case GLOBALDOC_OPTIONS:
        {
            DocGlobalConfigWidget *w1 = new DocGlobalConfigWidget(this, m_widget, page, "doc config widget");
            connect(dlg, SIGNAL(okClicked()), w1, SLOT(accept()));
            break;
        }
        case PROJECTDOC_OPTIONS:
        {
/*            DocProjectConfigWidget *w1 = new DocProjectConfigWidget(m_widget, page, project(), "doc project config");
            connect(dlg, SIGNAL(okClicked()), w1, SLOT(accept()));*/
            break;
        }
    }
}

KConfig *DocumentationPart::config()
{
    return DocumentationFactory::instance()->config();
}

bool DocumentationPart::configure()
{
    KDialogBase dlg(KDialogBase::Plain, i18n("Documentation Settings"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_widget,
                    "docsettings dialog");

    QVBoxLayout *l = new QVBoxLayout(dlg.plainPage(), 0, 0);
    DocGlobalConfigWidget *w1 = new DocGlobalConfigWidget(this, m_widget, dlg.plainPage());
    l->addWidget(w1);
    connect(&dlg, SIGNAL(okClicked()), w1, SLOT(accept()));
    return (dlg.exec() == QDialog::Accepted);
}

#include "documentation_part.moc"
