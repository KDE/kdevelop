/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_CHECKS_WIDGET_H
#define KDEVCLAZY_CHECKS_WIDGET_H

#include <QWidget>
#include <QHash>

class QTreeWidget;
class QTreeWidgetItem;

namespace Clazy
{

namespace Ui { class ChecksWidget; }

class ChecksDB;

class ChecksWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(
        QString checks
        READ checks
        WRITE setChecks
        NOTIFY checksChanged
        USER true)

public:
    explicit ChecksWidget(QWidget* parent = nullptr);
    ~ChecksWidget() override;

public:
    void setChecksDb(const QSharedPointer<const ChecksDB>& db);
    QString checks() const;

    void setChecks(const QString& checks);

    void setEditable(bool editable);

Q_SIGNALS:
    void checksChanged(const QString& checks);

private:
    void updateChecks();
    void setState(QTreeWidgetItem* item, Qt::CheckState state, bool force = true);
    void searchUpdated(const QString& searchString);

private:
    QScopedPointer<Ui::ChecksWidget> m_ui;

    QString m_checks;
    QHash<QString, QTreeWidgetItem*> m_items;
    bool m_isEditable = true;
};

}

#endif
