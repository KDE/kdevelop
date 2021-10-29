/*
    SPDX-FileCopyrightText: 2016 Zhigalin Alexander <alexander@zhigalin.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef COLORSCHEMECHOOSER_H
#define COLORSCHEMECHOOSER_H

#include <QAction>
#include <QtGlobal>

#include <kconfigwidgets_version.h>

#include "mainwindow.h"

namespace KDevelop
{
/**
 * Provides a menu that will offer to change the color scheme
 *
 * Furthermore, it will save the selection in the user configuration.
 */
class ColorSchemeChooser : public QAction
{
    Q_OBJECT

public:
    explicit ColorSchemeChooser(QObject* parent);

    QString currentSchemeName() const;
private Q_SLOTS:
    void slotSchemeChanged(QAction* triggeredAction);

private:
    QString loadCurrentScheme() const;
    void    saveCurrentScheme(const QString &name);
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 67, 0)
    QString currentDesktopDefaultScheme() const;
#endif
};

}  // namespace KDevelop

#endif // COLORSCHEMECHOOSER_H
