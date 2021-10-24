/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    void addSourceFormatterNoUi(KDevelop::ISourceFormatter* ifmt);
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

