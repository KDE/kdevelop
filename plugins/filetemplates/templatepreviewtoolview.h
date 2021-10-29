/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEWTOOLVIEW_H
#define KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEWTOOLVIEW_H

#include <QWidget>
#include <language/codegen/templaterenderer.h>

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
    explicit TemplatePreviewToolView(FileTemplatesPlugin* plugin, QWidget* parent);
    ~TemplatePreviewToolView() override;

private:
    Ui::TemplatePreviewToolView* ui;
    KTextEditor::Document* m_original;
    FileTemplatesPlugin* m_plugin;
    KDevelop::TemplateRenderer::EmptyLinesPolicy m_policy;

private Q_SLOTS:
    void sourceTextChanged(const QString& text);

protected:
    void showEvent(QShowEvent*) override;

public Q_SLOTS:
    void documentActivated(KDevelop::IDocument* document);
    void documentChanged(KTextEditor::Document* textDocument);
    void documentClosed(KDevelop::IDocument* document);
    void selectedRendererChanged();
};

#endif // KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEWTOOLVIEW_H
