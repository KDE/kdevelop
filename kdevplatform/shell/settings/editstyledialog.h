/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_EDITSTYLEDIALOG_H
#define KDEVPLATFORM_EDITSTYLEDIALOG_H

#include <QMimeType>
#include <QDialog>
#include <interfaces/isourceformatter.h>
#include "ui_editstyledialog.h"

namespace KTextEditor {
class Document;
class View;
}
namespace KDevelop {
class ISourceFormatter;
class SettingsWidget;
class SourceFormatterStyle;
}

/** \short A simple dialog to add preview around a \ref SettingsWidget
 */
class EditStyleDialog
    : public QDialog
{
    Q_OBJECT

public:
    /**
     * @pre @p formatter.hasEditStyleWidget() == @c true
     */
    EditStyleDialog(const KDevelop::ISourceFormatter& formatter, const QMimeType& mime,
                    const KDevelop::SourceFormatterStyle&, QWidget* parent = nullptr);
    ~EditStyleDialog() override;

    /** \return The string representing the style given by the \ref SettingsWidget.
     */
    QString content() const;

    /**
     * @return whether the style uses preview
     */
    bool usePreview() const;

private Q_SLOTS:
    void updatePreviewText(const QString &text);

private:
    void init();
    void initPreview();
    void showPreview(const QString& text);
    void previewVisibilityChanged(bool visible);

    const KDevelop::ISourceFormatter& m_sourceFormatter;
    KTextEditor::View* m_view = nullptr;
    KTextEditor::Document* m_document = nullptr;
    /// the settings widget becomes valid in constructor, never changes afterwards
    KDevelop::SettingsWidget* m_settingsWidget;
    QMimeType m_mimeType;
    QString m_pendingPreviewText; ///< if empty, m_document->text() is up to date
    Ui::EditStyle m_ui;
    KDevelop::SourceFormatterStyle m_style;
};

#endif // KDEVPLATFORM_EDITSTYLEDIALOG_H
