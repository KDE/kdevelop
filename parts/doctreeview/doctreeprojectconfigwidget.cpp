/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// KDE
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

// Local directory
#include "doctreeprojectconfigwidget.h"
#include "doctreeviewpart.h"
#include "doctreeviewwidget.h"
#include "doctreeviewfactory.h"
#include "misc.h"

// KDevelop
#include "kdevproject.h"
#include "domutil.h"


DocTreeProjectConfigWidget::DocTreeProjectConfigWidget(DocTreeViewWidget *widget, 
    QWidget *parent, KDevProject *project, const char *name): DocTreeProjectConfigWidgetBase(parent, name)

{
    m_widget = widget;
    m_project = project;

    readConfig();

    docListView->addColumn(i18n("Type"));
    docListView->addColumn(i18n("Enabled"));
    docListView->addColumn(i18n("Title"));
    docListView->addColumn(i18n("URL"));
    docListView->setAllColumnsShowFocus(true);
}

/*
DocTreeProjectConfigWidget::~DocTreeProjectConfigWidget()
{}
*/

void DocTreeProjectConfigWidget::readConfig()
{
    QMap<QString, QString> xmap;
    QDomDocument   d;
    KConfig*       config = DocTreeViewFactory::instance()->config();
    KStandardDirs* dirs   = DocTreeViewFactory::instance()->dirs();
    QStringList    tocs;
    QString        type;

    if (m_project->projectDom())
        d = *m_project->projectDom();

    QString userdocDir = DomUtil::readEntry(d ,
        "/kdevdoctreeview/projectdoc/userdocDir", m_project->projectDirectory() + "/html/" );
    userdocdirEdit->setURL( userdocDir );
    userdocdirEdit->fileDialog()->setMode( KFile::Directory );

    QString apidocDir = DomUtil::readEntry(d,
        "/kdevdoctreeview/projectdoc/apidocDir", m_project->projectDirectory() + "/html/" );
    apidocdirEdit->setURL( apidocDir );
    apidocdirEdit->fileDialog()->setMode( KFile::Directory );

    m_ignoreQT_XML  = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoreqt_xml", "toc");
    m_ignoreDoxygen = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoredoxygen", "toc");
    m_ignoreKDoc    = DomUtil::readListEntry(d, "/kdevdoctreeview/ignorekdocs", "toc");
    m_ignoreToc     = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoretocs", "toc");
    m_ignoreDevHelp = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoredevhelp", "toc");


    // Read qt *.xml Config ( DocType = QT_XML )
    type.setNum(QT_XML);
    xmap = config->entryMap("General Qt");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        if( m_ignoreQT_XML.contains( name ) )
            new KListViewItem(docListView, type, "false", itx.key(), config->readPathEntry(itx.key()));
        else
            new KListViewItem(docListView, type, "true",  itx.key(), config->readPathEntry(itx.key()));
    }

    // Read Doxygen Config ( DocType = DOXYGEN )
    type.setNum(DOXYGEN);
    xmap = config->entryMap("General Doxygen");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        if( m_ignoreDoxygen.contains( name ) )
            new KListViewItem(docListView, type, "false", itx.key(), config->readPathEntry(itx.key()));
        else
            new KListViewItem(docListView, type, "true",  itx.key(), config->readPathEntry(itx.key()));
    }


    // Read KDoc Config ( DocType = KDOC )
    type.setNum(KDOC);
    xmap = config->entryMap("General KDoc");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        if( m_ignoreKDoc.contains( name ) )
            new KListViewItem(docListView, type, "false", itx.key(), config->readPathEntry(itx.key()));
        else
            new KListViewItem(docListView, type, "true",  itx.key(), config->readPathEntry(itx.key()));
    }

    // Read Toc Config ( DocType = TOC )
    type.setNum(TOC);
    tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        const QString name( QFileInfo(*tit).baseName() );
        const QString location( DocTreeViewTool::tocLocation( *tit ) );
        const QString title (DocTreeViewTool::tocTitle( *tit ));
        if( m_ignoreToc.contains( name ) )
            new KListViewItem( docListView, type, "false", title, location);
        else
            new KListViewItem( docListView, type, "true",  title, location);
    }

    // Read DevHelp Config ( DocType = DEVHELP )
    type.setNum(DEVHELP);
    tocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        QFileInfo fi(*tit);
        BookInfo inf = DocTreeViewTool::devhelpInfo(*tit);
        if( m_ignoreDevHelp.contains( fi.baseName() ) )
            new KListViewItem( docListView, type, "false", inf.title, DocTreeViewTool::devhelpLocation(fi.baseName() , inf.defaultLocation));
        else
            new KListViewItem( docListView, type, "true",  inf.title, DocTreeViewTool::devhelpLocation(fi.baseName() , inf.defaultLocation));
    }
}

void DocTreeProjectConfigWidget::storeConfig()
{
    QDomDocument d;

    if (m_project->projectDom())
        d = *m_project->projectDom();

    DomUtil::writeEntry(d,
        "/kdevdoctreeview/projectdoc/userdocDir", userdocdirEdit->url());
    DomUtil::writeEntry(d,
        "/kdevdoctreeview/projectdoc/apidocDir", apidocdirEdit->url());

    DomUtil::writeListEntry(d,
        "/kdevdoctreeview/ignoreqt_xml", "toc", m_ignoreQT_XML );
    DomUtil::writeListEntry(d,
        "/kdevdoctreeview/ignoredoxygen", "toc", m_ignoreDoxygen );
    DomUtil::writeListEntry(d,
        "/kdevdoctreeview/ignorekdocs", "toc", m_ignoreKDoc );
    DomUtil::writeListEntry(d,
        "/kdevdoctreeview/ignoretocs", "toc", m_ignoreToc );
    DomUtil::writeListEntry(d,
        "/kdevdoctreeview/ignoredevhelp", "toc", m_ignoreDevHelp );
}

void DocTreeProjectConfigWidget::accept()
{
    storeConfig();
    m_widget->configurationChanged();
}

/*
void DocTreeProjectConfigWidget::setProject(KDevProject* project)
{
    readConfig();
    m_project = project;
}

*/

void DocTreeProjectConfigWidget::enable_clicked()
{
    QListViewItem *item( docListView->selectedItem() );
    if( item && item->text(1) == "false" )
    {
        switch (item->text(0).toInt())
        {
           case QT_XML:
                m_ignoreQT_XML.remove( item->text( 2 ) );
                item->setText(1, "true");
            break;

            case DOXYGEN:
                m_ignoreDoxygen.remove( item->text( 2 ) );
                item->setText(1, "true");
            break;

            case KDOC:
                m_ignoreKDoc.remove( item->text( 2 ) );
                item->setText(1, "true");
            break;

            case TOC:
                m_ignoreToc.remove( item->text( 2 ) );
                item->setText(1, "true");
            break;

            case DEVHELP:
                m_ignoreDevHelp.remove( item->text( 2 ) );
                item->setText(1, "true");
            break;

            default:
                kdDebug(9002) << "enable_clicked() called with unknown DocType: " << item->text(0) << endl;
        }
    }
}

void DocTreeProjectConfigWidget::disable_clicked()
{
    QListViewItem *item( docListView->selectedItem() );
    if( item && item->text(1) == "true" )
    {
        switch (item->text(0).toInt())
        {
            case QT_XML:
                m_ignoreQT_XML << item->text( 2 );
                item->setText(1, "false");
            break;

            case DOXYGEN:
                m_ignoreDoxygen << item->text( 2 );
                item->setText(1, "false");
            break;

            case KDOC:
                m_ignoreKDoc << item->text( 2 );
                item->setText(1, "false");
            break;

            case TOC:
                m_ignoreToc << item->text( 2 );
                item->setText(1, "false");
            break;

            case DEVHELP:
                m_ignoreDevHelp << item->text( 2 );
                item->setText(1, "false");
            break;

            default:
                kdDebug(9002) << "disable_clicked() called with unknown DocType: " << item->text(0) << endl;
        }
    }
}

#include "doctreeprojectconfigwidget.moc"
