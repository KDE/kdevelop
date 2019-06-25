/* This file is part of KDevelop
 *
 * Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
 * Copyright (C) 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_SOURCEFORMATTERSELECTIONEDIT_H
#define KDEVPLATFORM_SOURCEFORMATTERSELECTIONEDIT_H

#include <QWidget>

#include "shellexport.h"

class KConfigGroup;
class QListWidgetItem;

namespace KDevelop
{
class SourceFormatterStyle;
class ISourceFormatter;
class SourceFormatterSelectionEditPrivate;

class KDEVPLATFORMSHELL_EXPORT SourceFormatterSelectionEdit : public QWidget
{
    Q_OBJECT

public:
    explicit SourceFormatterSelectionEdit(QWidget* parent = nullptr);
    ~SourceFormatterSelectionEdit() override;

public:
    void loadSettings(const KConfigGroup& config);
    void saveSettings(KConfigGroup& config) const;

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void addSourceFormatter(KDevelop::ISourceFormatter* ifmt);
    void removeSourceFormatter(KDevelop::ISourceFormatter* ifmt);

    void deleteStyle();
    void editStyle();
    void newStyle();
    void selectLanguage(int );
    void selectFormatter(int );
    void selectStyle(int );
    void styleNameChanged(QListWidgetItem* );

private:
    void resetUi();
    void updatePreview();
    QListWidgetItem* addStyle(const KDevelop::SourceFormatterStyle& s);
    void enableStyleButtons();

private:
    const QScopedPointer<class SourceFormatterSelectionEditPrivate> d_ptr;
    Q_DECLARE_PRIVATE(SourceFormatterSelectionEdit)
};

}

#endif

