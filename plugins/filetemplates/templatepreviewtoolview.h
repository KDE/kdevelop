/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TEMPLATEPREVIEWTOOLVIEW_H
#define TEMPLATEPREVIEWTOOLVIEW_H

#include <QWidget>

namespace KTextEditor
{
class Document;
}

namespace KDevelop
{
class IDocument;
}

namespace Ui
{
class TemplatePreviewToolView;
}

class FileTemplatesPlugin;
class TemplatePreview;

class TemplatePreviewToolView : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatePreviewToolView(FileTemplatesPlugin* plugin, QWidget* parent, Qt::WindowFlags f = 0);
    virtual ~TemplatePreviewToolView();

private:
    Ui::TemplatePreviewToolView* ui;
    KTextEditor::Document* m_original;
    FileTemplatesPlugin* m_plugin;

private slots:
    void sourceTextChanged(const QString& text);

protected:
    virtual void showEvent(QShowEvent* event);

public slots:
    void documentActivated(KDevelop::IDocument* document);
    void documentChanged(KTextEditor::Document* textDocument);
    void documentClosed(KDevelop::IDocument* document);
    void selectedRendererChanged();
};

#endif // TEMPLATEPREVIEWTOOLVIEW_H
