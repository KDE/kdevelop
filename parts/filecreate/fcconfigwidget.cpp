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
#include <qcheckbox.h>
#include <klineedit.h>
#include <qlabel.h>

#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kurlrequester.h>
#include <kicondialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdeversion.h>

#include "fctypeedit.h"
#include "fctemplateedit.h"
#include "domutil.h"
#include "fcconfigwidget.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"


FCConfigWidget::FCConfigWidget(FileCreatePart * part, bool global, QWidget *parent, const char *name):
    FCConfigWidgetBase(parent, name), m_part(part), m_global(global)
{
    fc_view->setSorting(-1, FALSE);
    fcglobal_view->setSorting(-1, FALSE);

    if (m_global)
    {
        loadGlobalConfig(fc_view);
        fc_tabs->setTabLabel(tab1, i18n("Global Types") );
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
        templatesDir_label->setText(i18n("Project templates in ") + m_part->project()->projectDirectory() + "/templates");
    }

    m_globalfiletypes.setAutoDelete(true);
    m_projectfiletypes.setAutoDelete(true);
    m_projectfiletemplates.setAutoDelete(true);
}

FCConfigWidget::~FCConfigWidget()
{
}

void FCConfigWidget::accept()
{
    if (m_global) {
        saveGlobalConfig();
    }
    else
        saveProjectConfig();

    m_part->m_filetypes.clear();
    m_part->slotProjectOpened();

    for (QValueList<KURL>::iterator it = urlsToEdit.begin(); it != urlsToEdit.end(); ++it )
    {
        m_part->partController()->editDocument(*it);
    }
}

void FCConfigWidget::loadGlobalConfig(QListView *view, bool checkmarks)
{
    QString globalXMLFile = ::locate("data", "kdevfilecreate/template-info.xml");
    QDomDocument globalDom;
    if (!globalXMLFile.isNull() &&
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
                if (subtyperef.isNull())
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

    //check for new templates and those with location changed
    QListViewItemIterator it2(fctemplates_view);
    while (it2.current())
    {
        if (!it2.current()->text(1).isEmpty())
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

            if (!it.current()->text(4).isEmpty())
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

                if (!lastChild->text(4).isEmpty())
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
    if (templateUrl.isEmpty())
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
        if (!KIO::NetAccess::exists(destDir, false, 0 ))
            KIO::NetAccess::mkdir(destDir);

        KURL destination;
        destination.setPath(dest + destName);

        KIO::NetAccess::upload(templateUrl, destination);
    }
}

void FCConfigWidget::loadFileTypes(QPtrList<FileCreate::FileType> list, QListView *view, bool checkmarks)
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

void FCConfigWidget::removetemplate_button_clicked( )
{
    if (fctemplates_view->currentItem())
    {
        KURL removedTemplate;
        removedTemplate.setPath(m_part->project()->projectDirectory() + "/templates/" + fctemplates_view->currentItem()->text(0));
        KIO::NetAccess::del(removedTemplate);
        QListViewItem *it = fctemplates_view->currentItem();
        if (it->itemBelow())
        {
            fc_view->setSelected(it->itemBelow(), true);
            fc_view->setCurrentItem(it->itemBelow());
        }
        else if (it->itemAbove())
        {
            fc_view->setSelected(it->itemAbove(), true);
            fc_view->setCurrentItem(it->itemAbove());
        }
        delete it;
    }
}

void FCConfigWidget::copyToProject_button_clicked()
{
    QListViewItem *it = fcglobal_view->currentItem();
    if (it)
    {
        QListViewItem *it_copy_parent = 0;
        QString destParent;
        if (it->parent())
        {
            it_copy_parent = new QListViewItem(fc_view, it->parent()->text(0),
                it->parent()->text(1),
                it->parent()->text(2),
                it->parent()->text(3),
                locate("data", "kdevfilecreate/file-templates/"+ it->parent()->text(0)));
            destParent += it->parent()->text(0) + "-";
            QCheckListItem *chk = dynamic_cast<QCheckListItem*>(it->parent());
            if (chk)
                chk->setOn(false);
        }
        QListViewItem *it_copy = 0;
        if (it_copy_parent)
            it_copy = new QListViewItem(it_copy_parent, it->text(0),
                it->text(1),
                it->text(2),
                it->text(3),
                locate("data", "kdevfilecreate/file-templates/"+destParent + it->text(0)));
        else
            it_copy = new QListViewItem(fc_view, it->text(0),
                it->text(1),
                it->text(2),
                it->text(3),
                locate("data", "kdevfilecreate/file-templates/" +destParent+ it->text(0)));
        QCheckListItem *chk = dynamic_cast<QCheckListItem*>(it);
        if (chk)
            chk->setOn(false);
        fc_view->setSelected(it_copy, true);
        fc_view->setCurrentItem(it_copy);
        QListViewItem * it_child = it->firstChild();
        while( it_child ) {
            new QListViewItem(it_copy, it_child->text(0),
                it_child->text(1),
                it_child->text(2),
                it_child->text(3),
                locate("data", "kdevfilecreate/file-templates/"+ it_copy->text(0) + "-" + it_child->text(0)));
            QCheckListItem *chk_child = dynamic_cast<QCheckListItem*>(it_child);
            if (chk_child)
                chk_child->setOn(false);
            it_child = it_child->nextSibling();
        }
    }
}

void FCConfigWidget::newtype_button_clicked()
{
    FCTypeEdit *te = new FCTypeEdit();
    if (te->exec() == QDialog::Accepted )
    {
        QListViewItem *it = new QListViewItem(fc_view, te->typeext_edit->text(),
            te->typename_edit->text(),
            te->icon_url->icon(),
            te->typedescr_edit->text(),
            te->template_url->url().isEmpty() ? QString("create") : te->template_url->url());
        fc_view->setSelected(it, true);
        fc_view->setCurrentItem(it);
    }
    delete te;
}

void FCConfigWidget::newsubtype_button_clicked()
{
    if (fc_view->currentItem() && (!fc_view->currentItem()->parent()))
    {
        FCTypeEdit *te = new FCTypeEdit(this);
        if (te->exec() == QDialog::Accepted )
        {
            /*QListViewItem *it =*/(void) new QListViewItem(fc_view->currentItem(),
                te->typeext_edit->text(),
                te->typename_edit->text(),
                te->icon_url->icon(),
                te->typedescr_edit->text(),
                te->template_url->url().isEmpty() ? QString("create") : te->template_url->url());
            fc_view->currentItem()->setOpen(true);
        }
        delete te;
    }
}

void FCConfigWidget::remove_button_clicked()
{
    if (fc_view->currentItem())
    {
        QListViewItem *it = fc_view->currentItem();
        if (it->itemBelow())
        {
            fc_view->setSelected(it->itemBelow(), true);
            fc_view->setCurrentItem(it->itemBelow());
        }
        else if (it->itemAbove())
        {
            fc_view->setSelected(it->itemAbove(), true);
            fc_view->setCurrentItem(it->itemAbove());
        }
        delete it;
    }
}


void FCConfigWidget::moveup_button_clicked()
{
    QListViewItem *i = fc_view->currentItem();
    if ( !i )
        return;

    QListViewItemIterator it( i );
    QListViewItem *parent = i->parent();
    --it;
    while ( it.current() ) {
        if ( it.current()->parent() == parent )
            break;
        --it;
    }

    if ( !it.current() )
        return;
    QListViewItem *other = it.current();

    other->moveItem( i );
}


void FCConfigWidget::movedown_button_clicked()
{
    QListViewItem *i = fc_view->currentItem();
    if ( !i )
        return;

    QListViewItemIterator it( i );
    QListViewItem *parent = i->parent();
    it++;
    while ( it.current() ) {
        if ( it.current()->parent() == parent )
            break;
        it++;
    }

    if ( !it.current() )
        return;
    QListViewItem *other = it.current();

    i->moveItem( other );
}


void FCConfigWidget::edittype_button_clicked()
{
    QListViewItem *it = fc_view->currentItem();
    if ( it )
    {
        FCTypeEdit *te = new FCTypeEdit(this);

        te->typeext_edit->setText(it->text(0));
        te->typename_edit->setText(it->text(1));
        te->icon_url->setIcon(it->text(2));
        te->typedescr_edit->setText(it->text(3));
        if (it->text(4) != "create")
            te->template_url->setURL(it->text(4));

        if (te->exec() == QDialog::Accepted )
        {
            it->setText(0, te->typeext_edit->text());
            it->setText(1, te->typename_edit->text());
            it->setText(2, te->icon_url->icon());
            it->setText(3, te->typedescr_edit->text());
            if ((te->template_url->url() == "") && ((it->text(4) == "create")))
                it->setText(4, "create");
            else
                it->setText(4, te->template_url->url());
        }
    }
}


void FCConfigWidget::newtemplate_button_clicked()
{
    FCTemplateEdit *te = new FCTemplateEdit;
    if (te->exec() == QDialog::Accepted)
    {
        /*QListViewItem *it =*/(void) new QListViewItem(fctemplates_view, te->templatename_edit->text(),
            te->template_url->url().isEmpty() ? QString("create") : te->template_url->url());
    }
}


void FCConfigWidget::edittemplate_button_clicked()
{
    QListViewItem *it;
    if ( (it = fctemplates_view->currentItem()) )
    {
        FCTemplateEdit *te = new FCTemplateEdit;
        te->templatename_edit->setText(it->text(0));
        te->templatename_edit->setEnabled(false);
        if (te->exec() == QDialog::Accepted)
        {
            if ((te->template_url->url() == "") && ((it->text(1) == "create")))
                it->setText(1, "create");
            else
                it->setText(1, te->template_url->url());
        }
    }
}

void FCConfigWidget::edit_template_content_button_clicked( )
{
    if (fctemplates_view->currentItem())
    {
        QFileInfo fi(m_part->project()->projectDirectory() + "/templates/" + fctemplates_view->currentItem()->text(0));
        KURL content;
        content.setPath(m_part->project()->projectDirectory() + "/templates/" + fctemplates_view->currentItem()->text(0));
        if (fi.exists())
            m_part->partController()->editDocument(content);
        else
        {
            KMessageBox::information(this, i18n("Requested template does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString::null, "Edit template content warning");
            fctemplates_view->currentItem()->setPixmap(0, SmallIcon("edit"));
            urlsToEdit.append(content);
        }
    }
}

void FCConfigWidget::edit_type_content_button_clicked( )
{
    if (!fc_view->currentItem())
        return;
    QListViewItem *it = fc_view->currentItem();
    QString type_name = it->text(0);
    if (it->parent())
        type_name.prepend(it->parent()->text(0) + "-");
    if (!m_global)
    {
        QString typePath = m_part->project()->projectDirectory() + "/templates/" + type_name;
        KURL content;
        content.setPath(typePath);
        if (it->text(4).isEmpty())
            m_part->partController()->editDocument(content);
        else
        {
            if (it->text(4) == "create")
                KMessageBox::information(this, i18n("Template for the selected file type does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString::null, "Edit type template content warning");
            else
                KMessageBox::information(this, i18n("Template for the selected file type has been changed.\nIt will be opened immediately after accepting the configuration dialog."), QString::null, "Edit type template content warning");
            fc_view->currentItem()->setPixmap(0, SmallIcon("edit"));
            urlsToEdit.append(content);
        }
    }
    else
    {
        QString dest = KGlobal::dirs()->saveLocation("data", "/kdevfilecreate/file-templates/", true);
        QString typePath = dest + type_name;
        KURL content;
        content.setPath(typePath);
        if (it->text(4).isEmpty())
        {
            QFileInfo fi(dest+type_name);
            if (!fi.exists())
                copyTemplate(locate("data", "kdevfilecreate/file-templates/" + type_name), dest, type_name);
            m_part->partController()->editDocument(content);
        }
        else
        {
            if (it->text(4) == "create")
                KMessageBox::information(this, i18n("Template for the selected file type does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString::null, "Edit global type template content warning");
            else
                KMessageBox::information(this, i18n("Template for the selected file type has been changed.\nIt will be opened immediately after accepting the configuration dialog."), QString::null, "Edit global type template content warning");
            fc_view->currentItem()->setPixmap(0, SmallIcon("edit"));
            urlsToEdit.append(content);
        }
    }
}

#include "fcconfigwidget.moc"
