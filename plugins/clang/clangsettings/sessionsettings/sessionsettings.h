/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SESSIONSETTINGS_H
#define SESSIONSETTINGS_H

#include <interfaces/configpage.h>
#include "clangprivateexport.h"

#include <QScopedPointer>

namespace Ui
{
    class SessionSettings;
}

class KDEVCLANGPRIVATE_EXPORT SessionSettings: public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit SessionSettings(QWidget* parent);
    ~SessionSettings() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    void apply() override;
    void reset() override;

private:
    QScopedPointer<Ui::SessionSettings> m_settings;

};

#endif // SESSIONSETTINGS_H
