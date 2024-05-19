/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPCONFIG_H
#define QTHELPCONFIG_H

#include <interfaces/configpage.h>

namespace KNSCore {
class Entry;
}

class QTreeWidgetItem;
class QtHelpPlugin;

namespace Ui
{
    class QtHelpConfigUI;
}

class QtHelpConfig : public KDevelop::ConfigPage
{
public:
    Q_OBJECT

    public:
      explicit QtHelpConfig(QtHelpPlugin* plugin, QWidget *parent = nullptr);
      ~QtHelpConfig() override;

      KDevelop::ConfigPage::ConfigPageType configPageType() const override;

      bool checkNamespace(const QString &filename, QTreeWidgetItem* modifiedItem);

      QString name() const override;
      QString fullName() const override;
      QIcon icon() const override;

    private Q_SLOTS:
      void add();
      void remove(QTreeWidgetItem* item);
      void modify(QTreeWidgetItem* item);
      void knsUpdate(const QList<KNSCore::Entry>& list);

    public Q_SLOTS:
      void apply() override;
      void defaults() override;
      void reset() override;
    private:
      QTreeWidgetItem * addTableItem(const QString &icon, const QString &name,
                                     const QString &path, const QString &ghnsStatus);
      Ui::QtHelpConfigUI* m_configWidget;
};

#endif // QTHELPCONFIG_H
