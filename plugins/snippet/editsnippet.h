/***************************************************************************
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EDITSNIPPET_H
#define EDITSNIPPET_H

#include "ui_editsnippet.h"

#include <KDialog>

namespace KTextEditor
{
class Document;
class View;
}

class SnippetRepository;
class Snippet;

class QStandardItemModel;

/**
 * This dialog is used to create/edit snippets in a given repository.
 *
 * @author Milian Wolff <mail@milianw.de>
 */
class EditSnippet : public KDialog, protected Ui::EditSnippetBase
{
    Q_OBJECT

public:
    /// @p snippet set to 0 when you want to create a new snippet.
    explicit EditSnippet(SnippetRepository* repo, Snippet* snippet, QWidget* parent = 0);
    virtual ~EditSnippet();

    void setSnippetText(const QString& text);

private:
    SnippetRepository* m_repo;
    Snippet* m_snippet;
    KTextEditor::Document* m_document;
    KTextEditor::View* m_view;

private slots:
    void save();
    void validate();
};

#endif

