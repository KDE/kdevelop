/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "abbrevconfigwidget.h"

#include <kconfig.h>
#include <kiconloader.h>

#include <qlistview.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>

#include "addtemplatedlg.h"
#include "abbrevpart.h"

AbbrevConfigWidget::AbbrevConfigWidget(AbbrevPart *part, QWidget *parent, const char *name)
    : AbbrevConfigWidgetBase(parent, name)
{
    m_part = part;

    qWarning("creating abbrevconfigwidget for %d abbrevs", part->templates().allTemplates().count());
    QPtrList<CodeTemplate> templates = part->templates().allTemplates();
    CodeTemplate *templ;
    for (templ = templates.first(); templ; templ = templates.next())
    {
        qWarning("creating item for code template ");
        QListViewItem *it = new QListViewItem( listTemplates,
                           templ->name,
                           templ->description,
                           templ->suffixes,
                           templ->code,
                           templ->code );
        it->setPixmap( 0, SmallIcon("template_source"));
    }

    checkWordCompletion->setChecked( part->autoWordCompletionEnabled() );
    listTemplates->setSorting(2);
}


AbbrevConfigWidget::~AbbrevConfigWidget()
{}


void AbbrevConfigWidget::addTemplate()
{
    QStringList suffixesList = m_part->templates().suffixes();

    AddTemplateDialog dlg( suffixesList, this );
    if( dlg.exec() ){
        QString templ = dlg.templ();
        QString description = dlg.description();
        QString suffixes = dlg.suffixes();
        if( !(templ.isEmpty() || description.isEmpty()) || suffixes.isEmpty()) {
            QListViewItem* item = new QListViewItem( listTemplates, templ, description, suffixes );
            listTemplates->setSelected( item, true );
            editCode->setFocus();
        }
    }
}


void AbbrevConfigWidget::removeTemplate()
{
    if (!listTemplates->selectedItem())
        return;
    delete listTemplates->selectedItem();
}


void AbbrevConfigWidget::selectionChanged()
{
    QListViewItem* item = listTemplates->selectedItem();
    if( item ){
        editCode->setText( item->text(3) );
    }
}


void AbbrevConfigWidget::codeChanged()
{
    QListViewItem* item = listTemplates->selectedItem();
    if( item ){
        item->setText( 3, editCode->text() );
        if (item->text(3) == item->text(4))
            item->setPixmap( 0, SmallIcon("template_source") );
        else
            item->setPixmap( 0, SmallIcon("filesave") );
    }
}


void AbbrevConfigWidget::accept()
{
    m_part->clearTemplates();

    QListViewItem* item = listTemplates->firstChild();
    while( item ){
        m_part->addTemplate( item->text(0),
                             item->text(1),
                             item->text(2),
                             item->text(3) );
        item = item->nextSibling();
    }

    m_part->setAutoWordCompletionEnabled( checkWordCompletion->isChecked() );
}

#include "abbrevconfigwidget.moc"
