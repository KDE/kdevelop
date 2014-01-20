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

#include <interfaces/isourceformatter.h>
#include "ui_astyle_preferences.h"

// class AStylePlugin;
class AStyleFormatter;

class AStylePreferences : public KDevelop::SettingsWidget, public Ui::AStylePreferences
{
        Q_OBJECT

    public:
        enum Language { CPP, Java, CSharp};
        
        AStylePreferences(Language lang=CPP, QWidget *parent=0);
        virtual ~AStylePreferences();

        virtual void load(const KDevelop::SourceFormatterStyle &style);
        virtual QString save();

    protected:
        void init();
        void updatePreviewText(bool emitChangedSignal = true);
        void setItemChecked(int idx, bool checked);
        void updateWidgets();

    private slots:
        void currentTabChanged();
        void indentChanged();
        void indentObjectsChanged(QListWidgetItem *item);
        void minMaxValuesChanged();
        void bracketsChanged();
        void blocksChanged();
        void paddingChanged();
        void onelinersChanged();
        void pointerAlignChanged();

    private:
        AStyleFormatter *m_formatter;
        bool m_enableWidgetSignals;
};

#endif // ASTYLEPREFERENCES_H
