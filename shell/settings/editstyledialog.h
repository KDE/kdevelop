/* This file is part of KDevelop
 *  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

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
    EditStyleDialog(KDevelop::ISourceFormatter* formatter, const QMimeType& mime,
                    const KDevelop::SourceFormatterStyle&, QWidget* parent = 0);
    virtual ~EditStyleDialog();

    /** \return The string representing the style given by the \ref SettingsWidget.
     */
    QString content();

protected:
    void init();

public slots:
    void updatePreviewText(const QString &text);

private:
    KDevelop::ISourceFormatter* m_sourceFormatter;
    KTextEditor::View* m_view;
    KTextEditor::Document* m_document;
    KDevelop::SettingsWidget* m_settingsWidget;
    QMimeType m_mimeType;
    QWidget* m_content;
    Ui::EditStyle m_ui;
    KDevelop::SourceFormatterStyle m_style;
};

#endif // KDEVPLATFORM_EDITSTYLEDIALOG_H
