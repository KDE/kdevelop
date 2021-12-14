/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASTYLEPREFERENCES_H
#define ASTYLEPREFERENCES_H

#include <interfaces/isourceformatter.h>
#include "ui_astyle_preferences.h"

class AStyleFormatter;

class AStylePreferences : public KDevelop::SettingsWidget, public Ui::AStylePreferences
{
        Q_OBJECT

public:
    enum Language { CPP, Java, CSharp, ObjC};

    explicit AStylePreferences(Language lang=CPP, QWidget *parent=nullptr);
    ~AStylePreferences() override;

    void load(const KDevelop::SourceFormatterStyle &style) override;
    QString save() const override;

protected:
    void init();
    void updatePreviewText();
    void setItemChecked(int idx, bool checked);
    void updateWidgets();

private Q_SLOTS:
    void currentTabChanged();
    void indentChanged();
    void indentObjectsChanged(QListWidgetItem *item);
    void minMaxValuesChanged();
    void bracketsChanged();
    void blocksChanged();
    void paddingChanged();
    void onelinersChanged();
    void pointerAlignChanged();
    void afterParensChanged();

private:
    QScopedPointer<AStyleFormatter> m_formatter;
    bool m_enableWidgetSignals;
    const Language m_currentLanguage;
};

#endif // ASTYLEPREFERENCES_H
