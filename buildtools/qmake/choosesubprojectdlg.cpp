/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kpushbutton.h>

#include "trollprojectwidget.h"

#include "choosesubprojectdlg.h"
#include "scope.h"

ChooseSubprojectDlg::ChooseSubprojectDlg(TrollProjectWidget *widget, QWidget* parent, const char* name, bool modal, WFlags fl)
    : ChooseSubprojectDlgBase(parent,name, modal,fl), m_widget(widget)
{
    ChooseItem *it = new ChooseItem(m_widget->m_rootSubproject, subprojects_view, m_widget->m_rootSubproject->text(0));
    it->setPixmap(0, *(m_widget->m_rootSubproject->pixmap(0)));
    it->setOpen(true);
    fillSubprojectsView(it);

    connect(subprojects_view, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(itemSelected(QListViewItem *)));
    subprojects_view->setSelected(it, true);
}

ChooseSubprojectDlg::~ChooseSubprojectDlg()
{
}

/*$SPECIALIZATION$*/
void ChooseSubprojectDlg::accept()
{
    if (!subprojects_view->currentItem())
        return;
    ChooseItem *item = dynamic_cast<ChooseItem*>(subprojects_view->currentItem());
    if (!item)
        return;
    if ( item->subproject()->scope->variableValues("TEMPLATE").findIndex("subdirs") != -1 )
        return;

    QDialog::accept();
}

ChooseItem::ChooseItem( QMakeScopeItem * spitem, QListViewItem * parent, QString text )
    :KListViewItem(parent, text), m_spitem(spitem)
{
}

ChooseItem::ChooseItem( QMakeScopeItem * spitem, QListView * parent, QString text )
    :KListViewItem(parent, text), m_spitem(spitem)
{
}

QMakeScopeItem * ChooseItem::subproject( )
{
    return m_spitem;
}

void ChooseSubprojectDlg::fillSubprojectsView( ChooseItem *item )
{
    if (!item->subproject())
        return;

    QListViewItem * sub_spitem = item->subproject()->firstChild();
    while( sub_spitem ) {
        QMakeScopeItem *spitem = dynamic_cast<QMakeScopeItem *>(sub_spitem);
        if ( spitem && spitem->scope->scopeType() == Scope::ProjectScope )
        {
            ChooseItem *child_item = new ChooseItem(spitem, item, spitem->text(0));
            child_item->setPixmap(0, *(spitem->pixmap(0)));
            child_item->setOpen(true);
            fillSubprojectsView(child_item);
        }

        sub_spitem = sub_spitem->nextSibling();
    }
}

void ChooseSubprojectDlg::itemSelected( QListViewItem * it )
{
    if (!it)
        return;
    ChooseItem *item = dynamic_cast<ChooseItem*>(it);
    if (!item)
        return;
    if ( item->subproject()->scope->variableValues("TEMPLATE").findIndex("subdirs") != -1 )
        buttonOk->setEnabled(false);
    else
        buttonOk->setEnabled(true);
}

QMakeScopeItem * ChooseSubprojectDlg::selectedSubproject( )
{
    if (subprojects_view->currentItem())
    {
        ChooseItem *item = dynamic_cast<ChooseItem*>(subprojects_view->currentItem());
        if (item)
            return item->subproject();
    }

    return 0;
}


#include "choosesubprojectdlg.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

