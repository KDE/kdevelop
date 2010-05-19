/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editsnippet.h"

#include "snippetrepository.h"

#include <KLocalizedString>

#include <KTextEditor/EditorChooser>
#include <KPushButton>

#include "snippetstore.h"
#include "snippet.h"

EditSnippet::EditSnippet(SnippetRepository* repository, Snippet* snippet, QWidget* parent)
    : KDialog(parent), Ui::EditSnippetBase(), m_repo(repository), m_snippet(snippet)
{
    Q_ASSERT(m_repo);

    setButtons(/*Reset | */Apply | Cancel | Ok);
    setupUi(mainWidget());

    connect(this, SIGNAL(okClicked()), this, SLOT(save()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(save()));

    connect(snippetNameEdit, SIGNAL(textEdited(QString)), this, SLOT(validate()));
    connect(snippetContentsEdit, SIGNAL(textChanged()), this, SLOT(validate()));

    // if we edit a snippet, add all existing data
    if ( m_snippet ) {
        snippetNameEdit->setText(m_repo->text());

        setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));

        snippetArgumentsEdit->setText(m_snippet->arguments());
        snippetContentsEdit->setPlainText(m_snippet->snippet());
        snippetNameEdit->setText(m_snippet->text());
        snippetPostfixEdit->setText(m_snippet->postfix());
        snippetPrefixEdit->setText(m_snippet->prefix());
    } else {
        setWindowTitle(i18n("Create New Snippet in Repository %1", m_repo->text()));
    }

    validate();

    snippetNameEdit->setFocus();
}

EditSnippet::~EditSnippet()
{
}

void EditSnippet::validate()
{
    const QString& name = snippetNameEdit->text();
    bool valid = !name.isEmpty() && !snippetContentsEdit->document()->isEmpty();
    if (valid) {
        // make sure the snippetname includes no spaces
        for ( int i = 0; i < name.length(); ++i ) {
            if ( name.at(i).isSpace() ) {
                valid = false;
                break;
            }
        }
    }
    button(Ok)->setEnabled(valid);
    button(Apply)->setEnabled(valid);
}

void EditSnippet::save()
{
    Q_ASSERT(!snippetNameEdit->text().isEmpty());

    if ( !m_snippet ) {
        // save as new snippet
        m_snippet = new Snippet();
        m_repo->appendRow(m_snippet);
    }
    m_snippet->setArguments(snippetArgumentsEdit->text());
    m_snippet->setSnippet(snippetContentsEdit->document()->toPlainText());
    m_snippet->setText(snippetNameEdit->text());
    m_snippet->setPostfix(snippetPostfixEdit->text());
    m_snippet->setPrefix(snippetPrefixEdit->text());
    m_repo->save();

    setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));
}

#include "editsnippet.moc"
