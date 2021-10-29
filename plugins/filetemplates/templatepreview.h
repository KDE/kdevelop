/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEW_H
#define KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEW_H

#include <QWidget>
#include <QHash>

#include <language/codegen/templaterenderer.h>

namespace KTextEditor {
class Document;
class View;
}

/**
 * A renderer that adds some common variables for previewing purposes.
 */
class TemplatePreviewRenderer : public KDevelop::TemplateRenderer
{
public:
    TemplatePreviewRenderer();
    ~TemplatePreviewRenderer() override;
};

/**
 * A KTextEditor::View wrapper to show a preview of a template.
 */
class TemplatePreview : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePreview(QWidget* parent);
    ~TemplatePreview() override;

    /**
     * Set the template contents which will be rendered.
     *
     * @p text the template contents
     * @p isProject set to true if the contents resemble a project template
     * @return an error message, or an empty string if everything worked
     */
    QString setText(const QString& text, bool isProject = false,
                    KDevelop::TemplateRenderer::EmptyLinesPolicy policy = KDevelop::TemplateRenderer::TrimEmptyLines);

    /**
     * @return The read-only document.
     */
    KTextEditor::Document* document() const;

private:
    Q_DISABLE_COPY(TemplatePreview)

    QHash<QString, QString> m_variables;
    QScopedPointer<KTextEditor::Document> m_preview;
    KTextEditor::View* m_view;

};

#endif // KDEVPLATFORM_PLUGIN_TEMPLATEPREVIEW_H
