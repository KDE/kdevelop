/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <kurlrequester.h>
#include <kicondialog.h>

#include <qlistview.h>
#include <qlineedit.h>


#include "fctypeedit.h"
#include "fctemplateedit.h"


void FCConfigWidgetBase::newtype_button_clicked()
{
    FCTypeEdit *te = new FCTypeEdit();
    if (te->exec() == QDialog::Accepted )
    {
        QListViewItem *it = new QListViewItem(fc_view, te->typeext_edit->text(),
            te->typename_edit->text(),
            te->icon_url->icon(),
            te->typedescr_edit->text(),
            te->template_url->url().isNull() ? QString("create") : te->template_url->url());
        fc_view->setSelected(it, true);
        fc_view->setCurrentItem(it);
    }
    delete te;
}

void FCConfigWidgetBase::newsubtype_button_clicked()
{
    if (fc_view->currentItem() && (!fc_view->currentItem()->parent()))
    {
        FCTypeEdit *te = new FCTypeEdit(this);
        if (te->exec() == QDialog::Accepted )
        {
            QListViewItem *it = new QListViewItem(fc_view->currentItem(),
                te->typeext_edit->text(),
                te->typename_edit->text(),
                te->icon_url->icon(),
                te->typedescr_edit->text(),
                te->template_url->url().isNull() ? QString("create") : te->template_url->url());

            fc_view->currentItem()->setOpen(true);
        }
        delete te;
    }
}

void FCConfigWidgetBase::remove_button_clicked()
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


void FCConfigWidgetBase::moveup_button_clicked()
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


void FCConfigWidgetBase::movedown_button_clicked()
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


void FCConfigWidgetBase::moveleft_button_clicked()
{
}


void FCConfigWidgetBase::moveright_button_clicked()
{
}


void FCConfigWidgetBase::useglobal_button_clicked()
{
}


void FCConfigWidgetBase::edittype_button_clicked()
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



void FCConfigWidgetBase::newtemplate_button_clicked()
{
    FCTemplateEdit *te = new FCTemplateEdit;
    if (te->exec() == QDialog::Accepted)
    {
        QListViewItem *it = new QListViewItem(fctemplates_view, te->templatename_edit->text(), "create");
    }
}


void FCConfigWidgetBase::edittemplate_button_clicked()
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


void FCConfigWidgetBase::removetemplate_button_clicked()
{
    if (fctemplates_view->currentItem())
    {
        delete fctemplates_view->currentItem();
    }
}
