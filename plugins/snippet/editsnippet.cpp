/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editsnippet.h"

#include <kmessagebox.h>

#include "snippet.h"
#include "snippetstore.h"

EditSnippet::EditSnippet(Snippet *s, QWidget* parent)
 : QDialog(parent), Ui::EditSnippetBase(), snippet_(s)
{
    Ui::EditSnippetBase::setupUi(this);

    snippetName->setText( s->text() );
    snippetText->clear();
    snippetText->insertPlainText( s->getSnippetPlainText() );
    keywordList->addItems( s->getKeywordList() );
}

EditSnippet::~EditSnippet()
{
}

void EditSnippet::accept()
{
    QString name = snippetName->text();

    if ( name.isEmpty() ) {
        KMessageBox::error( this, i18n("Empty name"), i18n("Please enter a name") );
        return;
    }

    // First we need to save the user's data in our Snippet object
    snippet_->changeName( name );
    snippet_->setSnippetText( snippetText->toPlainText() );
    fillWithItemsFromListWidget( snippet_->getKeywordList() );

    // Write the Snippet's data back to it's file
    snippet_->save();

    // Call baseclass' accept() method
    QDialog::accept();
}

void EditSnippet::fillWithItemsFromListWidget(QStringList & list)
{
    list.clear();

    for (int i=0; i < keywordList->count(); i++) {
        QListWidgetItem* item  = keywordList->item(i);
        list << item->text();
    }
}

#include "editsnippet.moc"
// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
