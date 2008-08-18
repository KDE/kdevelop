/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

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
#ifndef ASTYLEPREFERENCES_H
#define ASTYLEPREFERENCES_H

#include <KCModule>

#include "ui_astyle_preferences.h"

namespace KTextEditor {
    class Document;
    class View;
}
class QListWidgetItem;

class AStylePlugin;
class AStyleFormatter;

class AStylePreferences : public KCModule, public Ui::AStylePreferences
{
    Q_OBJECT

    public:
        AStylePreferences(QWidget *parent, const QVariantList &args);
        virtual ~AStylePreferences();

    protected:
        void init();
        void updatePreviewText(bool emitChangedSignal = true);
        void addItemInStyleList(const QString &caption, const QString &name);
        void setItemChecked(int idx, bool checked);
        void updateWidgets();

    public slots:
        virtual void load();
        virtual void save();

    private slots:
        void currentStyleChanged(QListWidgetItem *current, QListWidgetItem *previous);
        void deleteCurrentStyle();
        void addStyle();
        void currentTabChanged();
        void indentChanged();
        void indentObjectsChanged(QListWidgetItem *item);
        void minMaxValuesChanged();
        void bracketsChanged();
        void blocksChanged();
        void paddingChanged();
        void onelinersChanged();

    private:
//         bool m_isGlobalWidget;
//         QString m_lastExt;
//         bool m_globalOptions;
        QString m_bracketSample, m_indentSample, m_formattingSample;
        QString m_fullSample;
        AStyleFormatter *m_formatter;
        KTextEditor::View *m_view;
        KTextEditor::Document *m_document;
        bool m_enableWidgetSignals;
};

#endif // ASTYLEPREFERENCES_H
