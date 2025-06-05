/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEPAGE_H
#define KDEVPLATFORM_TEMPLATEPAGE_H

#include <QWidget>

#include <memory>

namespace KDevelop
{
class ITemplateProvider;
}

class TemplatePagePrivate;

class TemplatePage : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePage(KDevelop::ITemplateProvider& templateProvider, QWidget* parent = nullptr);
    ~TemplatePage() override;

private:
    const std::unique_ptr<TemplatePagePrivate> d_ptr;
    Q_DECLARE_PRIVATE(TemplatePage)
};

#endif // KDEVPLATFORM_TEMPLATEPAGE_H
