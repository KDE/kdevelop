/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEPAGE_H
#define KDEVPLATFORM_TEMPLATEPAGE_H

#include <QWidget>

#include <memory>

class QModelIndex;
namespace KDevelop
{
class ITemplateProvider;
class TemplatesModel;
}

namespace Ui
{
    class TemplatePage;
}

class TreeViewTemplatesViewHelper;

class TemplatePage : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePage(KDevelop::ITemplateProvider* provider, QWidget* parent = nullptr);
    ~TemplatePage() override;

private Q_SLOTS:
    void currentIndexChanged(const QModelIndex& index);
    void extractTemplate();

private:
    [[nodiscard]] TreeViewTemplatesViewHelper viewHelper();

    /**
     * Call this function when refreshing the model invalidates the current index.
     */
    void currentIndexInvalidated();

    const std::unique_ptr<KDevelop::TemplatesModel> m_model;
    Ui::TemplatePage* ui;
};

#endif // KDEVPLATFORM_TEMPLATEPAGE_H
