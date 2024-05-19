/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEPAGE_H
#define KDEVPLATFORM_TEMPLATEPAGE_H

#include <QWidget>

class QModelIndex;
namespace KDevelop
{
class ITemplateProvider;
}

namespace Ui
{
    class TemplatePage;
}

namespace KNSCore {
class Entry;
}

class TemplatePage : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePage(KDevelop::ITemplateProvider* provider, QWidget* parent = nullptr);
    ~TemplatePage() override;

private Q_SLOTS:
    void loadFromFile();
    void handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries);
    void currentIndexChanged(const QModelIndex& index);
    void extractTemplate();

private:
    KDevelop::ITemplateProvider* m_provider;
    Ui::TemplatePage* ui;
};

#endif // KDEVPLATFORM_TEMPLATEPAGE_H
