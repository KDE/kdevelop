/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                 *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "abbrevconfigwidget.h"

#include <qlistview.h>
#include <qmultilineedit.h>

#include "addtemplatedlg.h"
#include "abbrevpart.h"


AbbrevConfigWidget::AbbrevConfigWidget(AbbrevPart *part, QWidget *parent, const char *name)
    : AbbrevConfigWidgetBase(parent, name)
{
    m_part = part;
    
    QAsciiDictIterator<CodeTemplate> it(part->templates());
    while( it.current() ){
        CodeTemplate* templ = it.current();
        new QListViewItem( listTemplates,
                           QString::fromLatin1(it.currentKey()),
                           templ->description,
                           templ->suffixes,
                           templ->code );
        ++it;
    }
}


AbbrevConfigWidget::~AbbrevConfigWidget()
{}


void AbbrevConfigWidget::addTemplate()
{
    QStringList suffixesList;
    QAsciiDictIterator<CodeTemplate> it(m_part->templates());
    for (; it.current(); ++it) {
        QString suffixes = it.current()->suffixes;
        if (!suffixesList.contains(suffixes))
            suffixesList << suffixes;
    }
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
}

#include "abbrevconfigwidget.moc"
