/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <q3listview.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtabwidget.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <Q3ValueList>

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
        sidetab_checkbox->setChecked(m_part->m_useSideTab);
    }
    else
    {
        loadGlobalConfig(fcglobal_view, true);
        loadProjectConfig(fc_view);
        loadProjectTemplates(fctemplates_view);
        sidetab_checkbox->setEnabled(false);
        templatesDir_label->setText(i18n("Project templates in ") + m_part->project()->projectDirectory() + "/templates");
    }

    m_globalfiletypes.setAutoDelete(true);
    m_projectfiletypes.setAutoDelete(true);
    m_projectfiletemplates.setAutoDelete(true);
//    connect( fctemplates_view, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ), this, SLOT( edittemplate_button_clicked() ) );
}

FCConfigWidget::~FCConfigWidget()
{
}

void FCConfigWidget::accept()
{
    if (m_global) {
        m_part->m_useSideTab = sidetab_checkbox->isChecked();
        m_part->setShowSideTab(m_part->m_useSideTab);
        saveGlobalConfig();
    }
    else
        saveProjectConfig();

    m_part->m_filetypes.clear();
    m_part->slotProjectOpened();

    for (Q3ValueList<KURL>::iterator it = urlsToEdit.begin(); it != urlsToEdit.end(); ++it )
    {
        m_part->partController()->editDocument(*it);
    }
}

void FCConfigWidget::loadGlobalConfig(Q3ListView *view, bool checkmarks)
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
                    Q3ListViewItem *it = view->findItem(ext, 0);
                    if (it)
                    {
                        ((Q3CheckListItem*)it)->setOn(true);

                        Q3ListViewItem *lastChild = it->firstChild();
                        while ( lastChild )
                        {
                            ((Q3CheckListItem*)lastChild)->setOn(true);
                            lastChild = lastChild->nextSibling();
                        }
                    }
                }
                else
                {
                    Q3ListViewItem *it = view->findItem(subtyperef, 0);
                    if (it)
                        ((Q3CheckListItem*)it)->setOn(true);
                }
            }
        }
    }
}

void FCConfigWidget::loadProjectConfig(Q3ListView *view)
{
    m_part->readTypes( *(m_part->projectDom()), m_projectfiletypes, false );
    loadFileTypes(m_projectfiletypes, view, false);
}

void FCConfigWidget::loadProjectTemplates(Q3ListView *view)
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
    QDomElement useST = globalDom.createElement("sidetab");
    useST.setAttribute("active", m_part->m_useSideTab ? "yes" : "no" );
    apPart.appendChild(useST);
    QDomElement fileTypes = globalDom.createElement( "filetypes" );
    apPart.appendChild( fileTypes );

    saveConfiguration(globalDom, fileTypes, true);

    QFile config( KGlobal::dirs()->saveLocation("data", "kdevfilecreate/", true) + "template-info.xml" );
    config.open(QIODevice::WriteOnly | QIODevice::Truncate);
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

    Q3ListViewItemIterator it( fcglobal_view );
    for( ; it.current( ); ++it ){
        if (!it.current()->parent())
        {
            Q3CheckListItem *chit = dynamic_cast<Q3CheckListItem*>(it.current());
            if ( !chit ) continue;
            if (chit->isOn())
            {
                QDomElement type = dom.createElement( "type" );
                type.setAttribute( "ext", chit->text(0) );
                globalTypes.appendChild( type );
            }
            else
            {
                Q3ListViewItem *lastChild = chit->firstChild();
                while ( lastChild )
                {
                    Q3CheckListItem *chsit = dynamic_cast<Q3CheckListItem*>(lastChild);
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
/*    QDir templDir( m_part->project()->projectDirectory() + "/templates/" );
    templDir.setFilter( QDir::Files );
    const QFileInfoList * list = templDir.entryInfoList();
    if( list )
    {
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
        while ( (fi = it.current()) != 0 )
        {
            if ( ( !(fctemplates_view->findItem(fi->fileName(), 0)) ) &&
                ( !(fc_view->findItem(fi->fileName(), 0)) ) )
            {
                KURL removedTemplate;
                removedTemplate.setPath(m_part->project()->projectDirectory() + "/templates/" + fi->fileName());
                KIO::NetAccess::del(removedTemplate);
            }
            ++it;
        }
    }*/
    //check for new templates and those with location changed
    Q3ListViewItemIterator it2(fctemplates_view);
    while (it2.current())
    {
        if (!it2.current()->text(1).isEmpty())
        {
            QString dest;
            dest = m_part->project()->projectDirectory() + "/templates/";
            if (it2.current()->text(1) == "create")
                copyTemplate(QString(), dest, it2.current()->text(0));
            else
                copyTemplate(it2.current()->text(1), dest, it2.current()->text(0));
        }
        ++it2;
    }
}

void FCConfigWidget::saveConfiguration(QDomDocument &dom, QDomElement &element, bool global)
{
    Q3ListViewItemIterator it( fc_view );
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
                    copyTemplate(QString(), dest, it.current()->text(0));
                else
                    copyTemplate(it.current()->text(4), dest, it.current()->text(0));
            }


            Q3ListViewItem *lastChild = it.current()->firstChild();
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
                        copyTemplate(QString(), dest, it.current()->text(0) + "-" + lastChild->text(0));
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
        f.open(QIODevice::WriteOnly);
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

void FCConfigWidget::loadFileTypes(Q3PtrList<FileCreate::FileType> list, Q3ListView *view, bool checkmarks)
{
    FileType *ft;

    for( ft = list.last(); ft; ft = list.prev())
    for( int i = list.count() - 1; i >= 0; --i)
    {
        if ( (ft = list.at(i)) )
        {
            Q3ListViewItem *it;
            if (!checkmarks)
                it = new Q3ListViewItem(view);
            else
                it = new Q3CheckListItem(view, "", Q3CheckListItem::CheckBox);

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
                    Q3ListViewItem *sit;
                    if (!checkmarks)
                        sit = new Q3ListViewItem(it);
                    else
                        sit = new Q3CheckListItem(it, "", Q3CheckListItem::CheckBox);

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
        Q3ListViewItem *it = fctemplates_view->currentItem();
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
    Q3ListViewItem *it = fcglobal_view->currentItem();
    if (it)
    {
        Q3ListViewItem *it_copy_parent = 0;
        QString destParent;
        if (it->parent())
        {
            it_copy_parent = new Q3ListViewItem(fc_view, it->parent()->text(0),
                it->parent()->text(1),
                it->parent()->text(2),
                it->parent()->text(3),
                locate("data", "kdevfilecreate/file-templates/"+ it->parent()->text(0)));
            destParent += it->parent()->text(0) + "-";
            Q3CheckListItem *chk = dynamic_cast<Q3CheckListItem*>(it->parent());
            if (chk)
                chk->setOn(false);
        }
        Q3ListViewItem *it_copy = 0;
        if (it_copy_parent)
            it_copy = new Q3ListViewItem(it_copy_parent, it->text(0),
                it->text(1),
                it->text(2),
                it->text(3),
                locate("data", "kdevfilecreate/file-templates/"+destParent + it->text(0)));
        else
            it_copy = new Q3ListViewItem(fc_view, it->text(0),
                it->text(1),
                it->text(2),
                it->text(3),
                locate("data", "kdevfilecreate/file-templates/" +destParent+ it->text(0)));
        Q3CheckListItem *chk = dynamic_cast<Q3CheckListItem*>(it);
        if (chk)
            chk->setOn(false);
        fc_view->setSelected(it_copy, true);
        fc_view->setCurrentItem(it_copy);
        Q3ListViewItem * it_child = it->firstChild();
        while( it_child ) {
            new Q3ListViewItem(it_copy, it_child->text(0),
                it_child->text(1),
                it_child->text(2),
                it_child->text(3),
                locate("data", "kdevfilecreate/file-templates/"+ it_copy->text(0) + "-" + it_child->text(0)));
            Q3CheckListItem *chk_child = dynamic_cast<Q3CheckListItem*>(it_child);
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
        Q3ListViewItem *it = new Q3ListViewItem(fc_view, te->typeext_edit->text(),
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
            /*QListViewItem *it =*/(void) new Q3ListViewItem(fc_view->currentItem(),
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
        Q3ListViewItem *it = fc_view->currentItem();
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
    Q3ListViewItem *i = fc_view->currentItem();
    if ( !i )
        return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    --it;
    while ( it.current() ) {
        if ( it.current()->parent() == parent )
            break;
        --it;
    }

    if ( !it.current() )
        return;
    Q3ListViewItem *other = it.current();

    other->moveItem( i );
}


void FCConfigWidget::movedown_button_clicked()
{
    Q3ListViewItem *i = fc_view->currentItem();
    if ( !i )
        return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    it++;
    while ( it.current() ) {
        if ( it.current()->parent() == parent )
            break;
        it++;
    }

    if ( !it.current() )
        return;
    Q3ListViewItem *other = it.current();

    i->moveItem( other );
}


void FCConfigWidget::edittype_button_clicked()
{
    Q3ListViewItem *it = fc_view->currentItem();
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
        /*QListViewItem *it =*/(void) new Q3ListViewItem(fctemplates_view, te->templatename_edit->text(),
            te->template_url->url().isEmpty() ? QString("create") : te->template_url->url());
    }
}


void FCConfigWidget::edittemplate_button_clicked()
{
    Q3ListViewItem *it;
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
            KMessageBox::information(this, i18n("Requested template does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString(), "Edit template content warning");
            fctemplates_view->currentItem()->setPixmap(0, SmallIcon("edit"));
            urlsToEdit.append(content);
        }
    }
}

void FCConfigWidget::edit_type_content_button_clicked( )
{
    if (!fc_view->currentItem())
        return;
    Q3ListViewItem *it = fc_view->currentItem();
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
                KMessageBox::information(this, i18n("Template for the selected file type does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString(), "Edit type template content warning");
            else
                KMessageBox::information(this, i18n("Template for the selected file type has been changed.\nIt will be opened immediately after accepting the configuration dialog."), QString(), "Edit type template content warning");
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
                KMessageBox::information(this, i18n("Template for the selected file type does not exist yet.\nIt will be opened immediately after accepting the configuration dialog."), QString(), "Edit global type template content warning");
            else
                KMessageBox::information(this, i18n("Template for the selected file type has been changed.\nIt will be opened immediately after accepting the configuration dialog."), QString(), "Edit global type template content warning");
            fc_view->currentItem()->setPixmap(0, SmallIcon("edit"));
            urlsToEdit.append(content);
        }
    }
}

#include "fcconfigwidget.moc"
