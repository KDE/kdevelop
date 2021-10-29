/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_LICENSEPAGE_H
#define KDEVPLATFORM_PLUGIN_LICENSEPAGE_H

#include <QWidget>

#include "ipagefocus.h"

namespace KDevelop {

/**
 * Assistant dialog page for choosing the license of new source files
 *
 * @todo Add the name of the Author at the top of the license
 */
class LicensePage : public QWidget, public IPageFocus
{
    Q_OBJECT

public:
    explicit LicensePage(QWidget* parent);
    ~LicensePage() override;

    /**
     * @return the full license header, which might either be a pre-defined
     *         or user-supplied one.
     */
    QString license() const;

    void setFocusToFirstEditWidget() override;

private:
    // data
    struct LicensePagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_LICENSEPAGE_H
