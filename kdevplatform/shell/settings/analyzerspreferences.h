/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ANALYZERS_PREFERENCES_H
#define KDEVPLATFORM_ANALYZERS_PREFERENCES_H

#include <interfaces/configpage.h>

namespace KDevelop
{

class AnalyzersPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit AnalyzersPreferences(QWidget* parent);
    ~AnalyzersPreferences() override;

    QString name() const override;
    QIcon icon() const override;
    QString fullName() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;
};

}

#endif
