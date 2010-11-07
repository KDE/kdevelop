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
#include <KPushButton>
#include <KAction>
#include <KMimeTypeTrader>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "snippetstore.h"
#include "snippet.h"

EditSnippet::EditSnippet(SnippetRepository* repository, Snippet* snippet, QWidget* parent)
    : KDialog(parent), Ui::EditSnippetBase(), m_repo(repository), m_snippet(snippet)
{
    Q_ASSERT(m_repo);

    setButtons(/*Reset | */Apply | Cancel | Ok);
    setupUi(mainWidget());

    KParts::ReadWritePart* part= KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadWritePart>(
                                        "text/plain", mainWidget(), mainWidget());
    m_document = qobject_cast<KTextEditor::Document*>(part);
    Q_ASSERT(m_document);
    Q_ASSERT(m_document->action("file_save"));
    m_document->action("file_save")->setEnabled(false);
    if (!m_repo->fileTypes().isEmpty()) {
        m_document->setMode(m_repo->fileTypes().first());
    }

    m_view = qobject_cast< KTextEditor::View* >( m_document->widget() );
    verticalLayout->addWidget(m_view, 5);
    snippetContentsLabel->setBuddy(m_view);

    verticalLayout->setMargin(0);
    formLayout->setMargin(0);

    snippetShortcutWidget->layout()->setMargin(0);

    connect(this, SIGNAL(okClicked()), this, SLOT(save()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(save()));

    connect(snippetNameEdit, SIGNAL(textEdited(QString)), this, SLOT(validate()));
    connect(m_document, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(validate()));

    // if we edit a snippet, add all existing data
    if ( m_snippet ) {
        snippetNameEdit->setText(m_repo->text());

        setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));

        snippetArgumentsEdit->setText(m_snippet->arguments());
        m_document->setText(m_snippet->snippet());
        snippetNameEdit->setText(m_snippet->text());
        snippetPostfixEdit->setText(m_snippet->postfix());
        snippetPrefixEdit->setText(m_snippet->prefix());
        snippetShortcutWidget->setShortcut(m_snippet->action()->shortcut());
    } else {
        setWindowTitle(i18n("Create New Snippet in Repository %1", m_repo->text()));
    }

    validate();

    snippetNameEdit->setFocus();

    QSize initSize = sizeHint();
    initSize.setHeight( initSize.height() + 200 );
    setInitialSize(initSize);
}

EditSnippet::~EditSnippet()
{
}

void EditSnippet::setSnippetText( const QString& text )
{
    m_document->setText(text);
    validate();
}

void EditSnippet::validate()
{
    const QString& name = snippetNameEdit->text();
    bool valid = !name.isEmpty() && !m_document->isEmpty();
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
    m_snippet->setSnippet(m_document->text());
    m_snippet->setText(snippetNameEdit->text());
    m_snippet->setPostfix(snippetPostfixEdit->text());
    m_snippet->setPrefix(snippetPrefixEdit->text());
    m_snippet->action()->setShortcut(snippetShortcutWidget->shortcut());
    m_repo->save();

    setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));
}

#include "editsnippet.moc"
