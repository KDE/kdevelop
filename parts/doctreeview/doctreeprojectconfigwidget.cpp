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


class DocCheckItem: public QCheckListItem
{
public:
    enum DocType
    {
        QT_XML,
        DOXYGEN,
        KDOC,
        TOC,
        DEVHELP
    };

    DocCheckItem ( DocTreeProjectConfigWidget *widget, DocType type, QCheckListItem * parent, const QString & text, Type tt = Controller )
        :QCheckListItem(parent, text, tt), m_widget(widget), m_type(type)
    {
    }

    DocCheckItem ( DocTreeProjectConfigWidget *widget, DocType type, QListViewItem * parent, const QString & text, Type tt = Controller )
        :QCheckListItem(parent, text, tt), m_widget(widget), m_type(type)
    {
    }

    DocCheckItem ( DocTreeProjectConfigWidget *widget, DocType type, QListView * parent, const QString & text, Type tt = Controller )
        :QCheckListItem(parent, text, tt), m_widget(widget), m_type(type)
    {
    }

    virtual DocType type()
    {
        return m_type;
    }

    virtual QString name() const
    {
        return m_name;
    }
    void setName(const QString &name)
    {
        m_name = name;
    }

protected:
    virtual void stateChange ( bool state )
    {
        if (state == true)
        {
            switch (type())
            {
            case QT_XML:
                    m_widget->m_ignoreQT_XML.remove( name() );
                break;

                case DOXYGEN:
                    m_widget->m_ignoreDoxygen.remove( name() );
                break;

                case KDOC:
                    m_widget->m_ignoreKDoc.remove( name() );
                break;

                case TOC:
                    m_widget->m_ignoreToc.remove( name() );
                break;

                case DEVHELP:
                    m_widget->m_ignoreDevHelp.remove( name() );
                break;

                default:
                    kdDebug(9002) << "item unchecked with unknown DocType: " << name() << endl;
            }
        }
        else
        {
            switch (type())
            {
                case QT_XML:
                    m_widget->m_ignoreQT_XML << name();
                break;

                case DOXYGEN:
                    m_widget->m_ignoreDoxygen << name();
                break;

                case KDOC:
                    m_widget->m_ignoreKDoc << name();
                break;

                case TOC:
                    m_widget->m_ignoreToc << name();
                break;

                case DEVHELP:
                    m_widget->m_ignoreDevHelp << name();
                break;

                default:
                    kdDebug(9002) << "item unchecked with unknown DocType: " << name() << endl;
            }
        }
    }

private:
    DocTreeProjectConfigWidget *m_widget;
    DocType m_type;
    QString m_name;
};


DocTreeProjectConfigWidget::DocTreeProjectConfigWidget(DocTreeViewWidget *widget,
    QWidget *parent, KDevProject *project, const char *name): DocTreeProjectConfigWidgetBase(parent, name)

{
    m_widget = widget;
    m_project = project;

    readConfig();

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

    if (m_project->projectDom())
        d = *m_project->projectDom();

    QString userdocDir = DomUtil::readEntry(d ,
        "/kdevdoctreeview/projectdoc/userdocDir", m_project->projectDirectory() + "/html/" );
    userdocdirEdit->setURL( userdocDir[0] != QChar('/') ? m_project->projectDirectory() + QString("/") + userdocDir : userdocDir );
    userdocdirEdit->fileDialog()->setMode( KFile::Directory );

    QString apidocDir = DomUtil::readEntry(d,
        "/kdevdoctreeview/projectdoc/apidocDir", m_project->projectDirectory() + "/html/" );
    apidocdirEdit->setURL( apidocDir[0] != QChar('/') ? m_project->projectDirectory() + QString("/") + apidocDir : apidocDir );
    apidocdirEdit->fileDialog()->setMode( KFile::Directory );

    m_ignoreQT_XML  = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoreqt_xml", "toc");
    m_ignoreDoxygen = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoredoxygen", "toc");
    m_ignoreKDoc    = DomUtil::readListEntry(d, "/kdevdoctreeview/ignorekdocs", "toc");
    m_ignoreToc     = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoretocs", "toc");
    m_ignoreDevHelp = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoredevhelp", "toc");

    m_qtDocs = new QListViewItem(docListView, i18n("Qt Documentation Collection"));
    m_qtDocs->setOpen(true);
    m_doxygenDocs = new QListViewItem(docListView, i18n("Doxygen Documentation Collection"));
    m_doxygenDocs->setOpen(true);
    m_kdocDocs = new QListViewItem(docListView, i18n("KDoc Documentation Collection"));
    m_kdocDocs->setOpen(true);
    m_tocDocs = new QListViewItem(docListView, i18n("KDevelopTOC Documentation Collection"));
    m_tocDocs->setOpen(true);
    m_devHelpDocs = new QListViewItem(docListView, i18n("DevHelp Documentation Collection"));
    m_devHelpDocs->setOpen(true);

    // Read qt *.xml Config ( DocType = QT_XML )
    xmap = config->entryMap("General Qt");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        DocCheckItem *item = new DocCheckItem(this, DocCheckItem::QT_XML, m_qtDocs, itx.key(), QCheckListItem::CheckBox);
        item->setText(1, config->readPathEntry(itx.key()));
        item->setName(name);
        item->setOn(!m_ignoreQT_XML.contains( name ));
    }

    // Read Doxygen Config ( DocType = DOXYGEN )
    xmap = config->entryMap("General Doxygen");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        DocCheckItem *item = new DocCheckItem(this, DocCheckItem::DOXYGEN, m_doxygenDocs, itx.key(), QCheckListItem::CheckBox);
        item->setText(1, config->readPathEntry(itx.key()));
        item->setName(name);
        item->setOn(!m_ignoreDoxygen.contains( name ));
    }


    // Read KDoc Config ( DocType = KDOC )
    xmap = config->entryMap("General KDoc");
    for (QMap<QString, QString>::Iterator itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        /// @todo Make it use a better name
        const QString name(itx.key());
        DocCheckItem *item = new DocCheckItem(this, DocCheckItem::KDOC, m_kdocDocs, itx.key(), QCheckListItem::CheckBox);
        item->setText(1, config->readPathEntry(itx.key()));
        item->setName(name);
        item->setOn(!m_ignoreKDoc.contains( name ));
    }

    // Read Toc Config ( DocType = TOC )
    tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        const QString name( QFileInfo(*tit).baseName() );
        const QString location( DocTreeViewTool::tocLocation( *tit ) );
        const QString title (DocTreeViewTool::tocTitle( *tit ));
        DocCheckItem *item = new DocCheckItem(this, DocCheckItem::TOC, m_tocDocs, title, QCheckListItem::CheckBox);
        item->setText(1, location);
        item->setName(name);
        item->setOn(!m_ignoreToc.contains( name ));
    }

    // Read DevHelp Config ( DocType = DEVHELP )
    tocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        const QString name( QFileInfo(*tit).baseName() );
        BookInfo inf = DocTreeViewTool::devhelpInfo(*tit);
        DocCheckItem *item = new DocCheckItem(this, DocCheckItem::DEVHELP, m_devHelpDocs, inf.title, QCheckListItem::CheckBox);
        item->setText(1, DocTreeViewTool::devhelpLocation(name, inf.defaultLocation));
        item->setName(name);
        item->setOn(!m_ignoreDevHelp.contains( name ));
    }
}

void DocTreeProjectConfigWidget::storeConfig()
{
    QDomDocument d;

    if (m_project->projectDom())
        d = *m_project->projectDom();

    QString userdocUrl = userdocdirEdit->url();
    QString apidocUrl = apidocdirEdit->url();

    if( userdocUrl.startsWith(m_project->projectDirectory()) )
        userdocUrl = userdocUrl.mid( m_project->projectDirectory().length() + 1 );

    if( apidocUrl.startsWith(m_project->projectDirectory()) )
        apidocUrl = apidocUrl.mid( m_project->projectDirectory().length() + 1 );

    DomUtil::writeEntry(d,
        "/kdevdoctreeview/projectdoc/userdocDir", userdocUrl );
    DomUtil::writeEntry(d,
        "/kdevdoctreeview/projectdoc/apidocDir", apidocUrl );

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

#include "doctreeprojectconfigwidget.moc"
