/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qlistview.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtabwidget.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "domutil.h"
#include "fcconfigwidget.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"
#include "kdevproject.h"

using namespace FileCreate;

FCConfigWidget::FCConfigWidget(FileCreatePart * part, bool global, QWidget *parent, const char *name):
    FCConfigWidgetBase(parent, name), m_part(part), m_global(global)
{
    fc_view->setSorting(-1, FALSE);
    fcglobal_view->setSorting(-1, FALSE);

    if (m_global)
    {
        loadGlobalConfig(fc_view);
        fc_tabs->setTabLabel(tab1, "Global Types");
        fc_tabs->setTabEnabled(tab2, false);
        fc_tabs->setTabEnabled(tab3, false);
        delete tab2;
        delete tab3;
    }
    else
    {
        loadGlobalConfig(fcglobal_view, true);
        loadProjectConfig(fc_view);
        loadProjectTemplates(fctemplates_view);
    }
}

FCConfigWidget::~FCConfigWidget()
{
}

void FCConfigWidget::accept()
{
    if (m_global)
        saveGlobalConfig();
    else
        saveProjectConfig();

    m_part->m_filetypes.clear();
    m_part->slotProjectOpened();
}

void FCConfigWidget::loadGlobalConfig(QListView *view, bool checkmarks)
{
    QString globalXMLFile = ::locate("data", "kdevfilecreate/template-info.xml");
    QDomDocument globalDom;
    if (globalXMLFile!=QString::null &&
        DomUtil::openDOMFile(globalDom,globalXMLFile)) {
        m_part->readTypes(globalDom, m_globalfiletypes, false);
    }


    loadFileTypes(m_globalfiletypes, view, checkmarks);

    if (checkmarks)
    {
        QDomElement useGlobalTypes =
            DomUtil::elementByPath(*(m_part->projectDom()),"/kdevfilecreate/useglobaltypes");
        for(QDomNode node = useGlobalTypes.firstChild(); !node.isNull();node=node.nextSibling())
        {
            if (node.isElement() && node.nodeName()=="type")
            {
                QDomElement element = node.toElement();
                QString ext = element.attribute("ext");
                QString subtyperef = element.attribute("subtyperef");
                if (subtyperef==QString::null)
                {
                    QListViewItem *it = view->findItem(ext, 0);
                    if (it)
                    {
                        ((QCheckListItem*)it)->setOn(true);

                        QListViewItem *lastChild = it->firstChild();
                        while ( lastChild )
                        {
                            ((QCheckListItem*)lastChild)->setOn(true);
                            lastChild = lastChild->nextSibling();
                        }
                    }
                }
                else
                {
                    QListViewItem *it = view->findItem(subtyperef, 0);
                    if (it)
                        ((QCheckListItem*)it)->setOn(true);
                }
            }
        }
    }
}

void FCConfigWidget::loadProjectConfig(QListView *view)
{
    m_part->readTypes( *(m_part->projectDom()), m_projectfiletypes, false );
    loadFileTypes(m_projectfiletypes, view, false);
}

void FCConfigWidget::loadProjectTemplates(QListView *view)
{
    QDir templDir( m_part->project()->projectDirectory() + "/templates/" );
    templDir.setFilter( QDir::Files );
    const QFileInfoList * list = templDir.entryInfoList();
    if( list ){
      QFileInfoListIterator it( *list );
      QFileInfo *fi;
      while ( (fi = it.current()) != 0 ) {
        FileType * filetype;
        filetype = new FileType;
        //name shall be "" for proper configuration
        filetype->setName( "" );
        filetype->setExt( fi->fileName() );
        m_projectfiletemplates.append(filetype);
        filetype->setEnabled(false);
        ++it;
      }
    }
    loadFileTypes(m_projectfiletemplates, view, false);
}


void FCConfigWidget::saveGlobalConfig()
{
    QDomDocument globalDom;
    QDomElement  element = globalDom.createElement("kdevelop" );
    globalDom.appendChild(element);
    QDomElement  apPart  = globalDom.createElement("kdevfilecreate");
    element.appendChild(apPart);
    QDomElement fileTypes = globalDom.createElement( "filetypes" );
    apPart.appendChild( fileTypes );

    saveConfiguration(globalDom, fileTypes, true);

    QFile config( KGlobal::dirs()->saveLocation("data", "kdevfilecreate/", true) + "template-info.xml" );
    config.open(IO_WriteOnly | IO_Truncate);
    QTextStream stream(&config);
    stream << "<?xml version = '1.0'?>";
    stream << globalDom.toString();
    config.close();
}

void FCConfigWidget::saveProjectConfig()
{
    QDomDocument dom     = *m_part->projectDom( );
    QDomElement  element = dom.documentElement( );
    QDomElement  apPart  = element.namedItem( "kdevfilecreate" ).toElement( );
    if( apPart.isNull( ) )
    {
        apPart = dom.createElement( "kdevfilecreate" );
        element.appendChild( apPart );
    }


    // project template configuration

    QDomElement projectTypes = apPart.namedItem( "filetypes" ).toElement( );
    apPart.removeChild(projectTypes);
    projectTypes = dom.createElement( "filetypes" );
    apPart.appendChild( projectTypes );

    saveConfiguration(dom, projectTypes, false);


    // global template usage

    QDomElement globalTypes = apPart.namedItem( "useglobaltypes" ).toElement( );
    apPart.removeChild(globalTypes);
    globalTypes = dom.createElement( "useglobaltypes" );
    apPart.appendChild( globalTypes );

    QListViewItemIterator it( fcglobal_view );
    for( ; it.current( ); ++it ){
        if (!it.current()->parent())
        {
            QCheckListItem *chit = dynamic_cast<QCheckListItem*>(it.current());
            if ( !chit ) continue;
            if (chit->isOn())
            {
                QDomElement type = dom.createElement( "type" );
                type.setAttribute( "ext", chit->text(0) );
                globalTypes.appendChild( type );
            }
            else
            {
                QListViewItem *lastChild = chit->firstChild();
                while ( lastChild )
                {
                    QCheckListItem *chsit = dynamic_cast<QCheckListItem*>(lastChild);
                    if ( (chsit) && (chsit->isOn()))
                    {
                        QDomElement type = dom.createElement( "type" );
                        type.setAttribute( "ext", chit->text(0) );
                        type.setAttribute( "subtyperef", chsit->text(0) );
                        globalTypes.appendChild( type );
                    }

                    lastChild = lastChild->nextSibling();
                }
            }
        }
    }


    // project template files

    //check for removed templates
    QDir templDir( m_part->project()->projectDirectory() + "/templates/" );
    templDir.setFilter( QDir::Files );
    const QFileInfoList * list = templDir.entryInfoList();
    if( list )
    {
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
        while ( (fi = it.current()) != 0 )
        {
            if ( !(fctemplates_view->findItem(fi->fileName(), 0)) )
            {
                KURL removedTemplate;
                removedTemplate.setPath(m_part->project()->projectDirectory() + "/templates/" + fi->fileName());
                KIO::NetAccess::del(removedTemplate);
            }
            ++it;
        }
    }
    //check for new templates and those with location changed
    QListViewItemIterator it2(fctemplates_view);
    while (it2.current())
    {
        if (it2.current()->text(1) != "")
        {
            QString dest;
            dest = m_part->project()->projectDirectory() + "/templates/";
            if (it2.current()->text(1) == "create")
                copyTemplate(QString::null, dest, it2.current()->text(0));
            else
                copyTemplate(it2.current()->text(1), dest, it2.current()->text(0));
        }
        ++it2;
    }
}

void FCConfigWidget::saveConfiguration(QDomDocument &dom, QDomElement &element, bool global)
{
    QListViewItemIterator it( fc_view );
    for( ; it.current( ); ++it ){
        if (!it.current()->parent())
        {
            QDomElement type = dom.createElement( "type" );
            type.setAttribute( "ext", it.current()->text(0) );
            type.setAttribute( "name", it.current()->text(1) );
            if (it.current()->childCount() > 0)
                type.setAttribute( "create", "no");
            else
                type.setAttribute( "create", "template");
            type.setAttribute( "icon", it.current()->text(2) );

            QDomElement edescr = dom.createElement("descr");
            type.appendChild(edescr);
            QDomText descr = dom.createTextNode( it.current()->text(3) );
            edescr.appendChild(descr);

            if (it.current()->text(4) != "")
            {
                QString dest;
                if (global)
                    dest = KGlobal::dirs()->saveLocation("data", "/kdevfilecreate/file-templates/", true);
                else
                    dest = m_part->project()->projectDirectory() + "/templates/";
                if (it.current()->text(4) == "create")
                    copyTemplate(QString::null, dest, it.current()->text(0));
                else
                    copyTemplate(it.current()->text(4), dest, it.current()->text(0));
            }


            QListViewItem *lastChild = it.current()->firstChild();
            while ( lastChild )
            {
                QDomElement subtype = dom.createElement( "subtype" );
                subtype.setAttribute( "ref", lastChild->text(0) );
                subtype.setAttribute( "name", lastChild->text(1) );
                subtype.setAttribute( "icon", lastChild->text(2) );

                QDomElement edescr = dom.createElement("descr");
                subtype.appendChild(edescr);
                QDomText descr = dom.createTextNode( lastChild->text(3) );
                edescr.appendChild(descr);

                if (lastChild->text(4) != "")
                {
                    QString dest;
                    if (global)
                        dest = KGlobal::dirs()->saveLocation("data", "/kdevfilecreate/file-templates/", true);
                    else
                        dest = m_part->project()->projectDirectory() + "/templates/";
                    if (lastChild->text(4) == "create")
                        copyTemplate(QString::null, dest, it.current()->text(0) + "-" + lastChild->text(0));
                    else
                        copyTemplate(lastChild->text(4), dest, it.current()->text(0) + "-" + lastChild->text(0));
                }

                type.appendChild( subtype );
                lastChild = lastChild->nextSibling();
            }

            element.appendChild( type );
        }
    }
}

void FCConfigWidget::copyTemplate(QString templateUrl, QString dest, QString destName)
{
    if (templateUrl == "")
    {
        QDir d(dest);
        if (!d.exists())
            d.mkdir(dest);

        QFile f(dest + destName);
        f.open(IO_WriteOnly);
        f.close();
    }
    else
    {
        KURL destDir;
        destDir.setPath(dest);
        if (!KIO::NetAccess::exists(destDir))
            KIO::NetAccess::mkdir(destDir);

        KURL destination;
        destination.setPath(dest + destName);

        KIO::NetAccess::upload(templateUrl, destination);
    }
}

void FCConfigWidget::loadFileTypes(QPtrList<FileType> list, QListView *view, bool checkmarks)
{
    FileType *ft;

    for( ft = list.last(); ft; ft = list.prev())
    for( int i = list.count() - 1; i >= 0; --i)
    {
        if ( (ft = list.at(i)) )
        {
            QListViewItem *it;
            if (!checkmarks)
                it = new QListViewItem(view);
            else
                it = new QCheckListItem(view, "", QCheckListItem::CheckBox);

            it->setText(0, ft->ext());
            it->setText(1, ft->name());
            it->setText(2, ft->icon());
            it->setText(3, ft->descr());
            it->setText(4, "");

            FileType *sft;
            for( int j = ft->subtypes().count() - 1; j >= 0; --j)
            {
                if ( (sft = ft->subtypes().at(j)) )
                {
                    QListViewItem *sit;
                    if (!checkmarks)
                        sit = new QListViewItem(it);
                    else
                        sit = new QCheckListItem(it, "", QCheckListItem::CheckBox);

                    sit->setText(0, sft->subtypeRef());
                    sit->setText(1, sft->name());
                    sit->setText(2, sft->icon());
                    sit->setText(3, sft->descr());
                    sit->setText(4, "");
                }
            }
        }
    }
}

