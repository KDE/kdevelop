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

#ifndef TEMPLATEPREVIEW_H
#define TEMPLATEPREVIEW_H

#include <QWidget>
#include <QHash>

namespace KTextEditor
{
class Document;
}

namespace KDevelop
{
class TemplateRenderer;
class IDocument;
}

namespace Ui
{
class TemplatePreview;
}

class KTemporaryFile;


class TemplatePreview : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatePreview(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~TemplatePreview();

private:
    Ui::TemplatePreview* ui;
    KDevelop::TemplateRenderer* m_renderer;
    QHash<QString,QString> m_variables;
    KTextEditor::Document* m_original;
    KTextEditor::Document* m_preview;

private slots:
    void sourceTextChanged(const QString& text);

public slots:
    void documentActivated(KDevelop::IDocument* document);
    void documentChanged(KTextEditor::Document* textDocument);
    void documentClosed(KDevelop::IDocument* document);
    void selectedRendererChanged();
};

#endif // TEMPLATEPREVIEW_H
