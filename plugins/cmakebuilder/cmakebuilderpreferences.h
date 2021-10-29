/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEBUILDERPREFERENCES_H
#define CMAKEBUILDERPREFERENCES_H

#include <interfaces/configpage.h>

class QWidget;
class CMakeBuilderSettings;
namespace Ui { class CMakeBuilderPreferences; }

class CMakeBuilderPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit CMakeBuilderPreferences(KDevelop::IPlugin* plugin, QWidget* parent = nullptr);
    ~CMakeBuilderPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

private:
    Ui::CMakeBuilderPreferences* m_prefsUi;
};

#endif
