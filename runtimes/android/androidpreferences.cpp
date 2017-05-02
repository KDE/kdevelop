/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "androidpreferences.h"
#include "ui_androidpreferences.h"

AndroidPreferences::AndroidPreferences(KDevelop::IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent)
    : KDevelop::ConfigPage(plugin, config, parent)
{
    auto m_prefsUi = new Ui::AndroidPreferences;
    m_prefsUi->setupUi(this);
}

AndroidPreferences::~AndroidPreferences() = default;

QString AndroidPreferences::name() const
{
    return QStringLiteral("Android");
}
