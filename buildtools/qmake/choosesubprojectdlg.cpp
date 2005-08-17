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

ChooseSubprojectDlg::ChooseSubprojectDlg(TrollProjectWidget *widget, QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : ChooseSubprojectDlgBase(parent,name, modal,fl), m_widget(widget)
{
    ChooseItem *it = new ChooseItem(m_widget->m_rootSubproject, subprojects_view, m_widget->m_rootSubproject->text(0));
    it->setPixmap(0, *(m_widget->m_rootSubproject->pixmap(0)));
    it->setOpen(true);
    fillSubprojectsView(it);

    connect(subprojects_view, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(itemSelected(Q3ListViewItem *)));
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
    if (item->subproject()->configuration.m_template == QTMP_SUBDIRS)
        return;

    QDialog::accept();
}

ChooseItem::ChooseItem( SubqmakeprojectItem * spitem, Q3ListViewItem * parent, QString text )
    :KListViewItem(parent, text), m_spitem(spitem)
{
}

ChooseItem::ChooseItem( SubqmakeprojectItem * spitem, Q3ListView * parent, QString text )
    :KListViewItem(parent, text), m_spitem(spitem)
{
}

SubqmakeprojectItem * ChooseItem::subproject( )
{
    return m_spitem;
}

void ChooseSubprojectDlg::fillSubprojectsView( ChooseItem *item )
{
    if (!item->subproject())
        return;

    Q3ListViewItem * sub_spitem = item->subproject()->firstChild();
    while( sub_spitem ) {
        SubqmakeprojectItem *spitem = dynamic_cast<SubqmakeprojectItem *>(sub_spitem);
        if (spitem)
        {
            ChooseItem *child_item = new ChooseItem(spitem, item, spitem->text(0));
            child_item->setPixmap(0, *(spitem->pixmap(0)));
            child_item->setOpen(true);
            fillSubprojectsView(child_item);
        }

        sub_spitem = sub_spitem->nextSibling();
    }
}

void ChooseSubprojectDlg::itemSelected( Q3ListViewItem * it )
{
    if (!it)
        return;
    ChooseItem *item = dynamic_cast<ChooseItem*>(it);
    if (!item)
        return;
    if (item->subproject()->configuration.m_template == QTMP_SUBDIRS)
        buttonOk->setEnabled(false);
    else
        buttonOk->setEnabled(true);
}

SubqmakeprojectItem * ChooseSubprojectDlg::selectedSubproject( )
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

