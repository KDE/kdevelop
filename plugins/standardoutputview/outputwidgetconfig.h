/*
    SPDX-FileCopyrightText: 2023 George Florea Bănuș <georgefb899@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OUTPUTWIDGETCONFIG_H
#define OUTPUTWIDGETCONFIG_H

#include <QObject>

#include <optional>

class KConfigGroup;
class QWidget;

class OutputWidgetConfig : public QObject
{
    Q_OBJECT
public:
    explicit OutputWidgetConfig(const QString& configSubgroupName, const QString& toolViewTitle, QObject* parent);

    /// @return Maximum number of open output views or std::nullopt if unlimited.
    std::optional<int> maxViewCount() const;

    void openDialog(QWidget* dialogParent);

Q_SIGNALS:
    void settingsChanged();

private:
    KConfigGroup configSubgroup() const;
    int maxViewCount(const KConfigGroup& configSubgroup) const;
    bool isViewLimitEnabled(const KConfigGroup& configSubgroup) const;

    const QString m_configSubgroupName;
    const QString m_toolViewTitle;
};

#endif // OUTPUTWIDGETCONFIG_H
