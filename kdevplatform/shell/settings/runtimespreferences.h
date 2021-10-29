/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_RUNTIMES_PREFERENCES_H
#define KDEVPLATFORM_RUNTIMES_PREFERENCES_H

#include <interfaces/configpage.h>

namespace KDevelop
{

class RuntimesPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit RuntimesPreferences(QWidget* parent);
    ~RuntimesPreferences() override;

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
