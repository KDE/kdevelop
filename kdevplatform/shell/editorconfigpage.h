/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEVPLATFORM_EDITORCONFIGPAGE_H
#define KDEVPLATFORM_EDITORCONFIGPAGE_H

#include <interfaces/configpage.h>

namespace KDevelop {

/**
 * This class makes all the KTextEditor config pages available for use
 * as a single KDevelop::ConfigPage
 */
class EditorConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit EditorConfigPage(QWidget* parent);
    ~EditorConfigPage() override;

    QString name() const override;
    QIcon icon() const override;
    QString fullName() const override;
    int childPages() const override;
    ConfigPage* childPage(int number) override;
public Q_SLOTS:
    // nothing to edit on this page, only subpages have actual configuration
    void apply() override {};
    void reset() override {};
    void defaults() override {};
};
}

#endif // KDEVPLATFORM_EDITORCONFIGPAGE_H
