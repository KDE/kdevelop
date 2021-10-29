/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebaukde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CUSTOMCHECKSETCONFIGPROXYWIDGET_H
#define CLANGTIDY_CUSTOMCHECKSETCONFIGPROXYWIDGET_H

#include <QWidget>

namespace ClangTidy
{

/**
 * Dummy widget to integrate processing of the custom checks with KConfigDialogManager
 * It's only purpose is to be part of the config form widget hierarchy and by that being picked up
 * by KConfigDialogManager scanning widgets for those with kcfg_* object names, to use them for
 * reading & writing values of the matching items of the ConfigSkeleton.
 */
class CustomCheckSetConfigProxyWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(
        QString checks
        READ checks
        WRITE setChecks
        NOTIFY checksChanged
        USER true)

public:
    explicit CustomCheckSetConfigProxyWidget(QWidget* parent = nullptr);
    ~CustomCheckSetConfigProxyWidget() override;

public:
    QString checks() const;

    void setChecks(const QString& checks);

Q_SIGNALS:
    void checksChanged(const QString& checks);

private:
    QString m_checks;
};

}

#endif
