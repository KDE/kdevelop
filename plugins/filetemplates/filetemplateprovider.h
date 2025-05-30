/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_FILE_TEMPLATE_PROVIDER_H
#define KDEVPLATFORM_PLUGIN_FILE_TEMPLATE_PROVIDER_H

#include <interfaces/itemplateprovider.h>

class FileTemplateProvider : public KDevelop::ITemplateProvider
{
public:
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] std::unique_ptr<KDevelop::TemplatesModel> createTemplatesModel() const override;
    [[nodiscard]] QString knsConfigurationFile() const override;
};

#endif // KDEVPLATFORM_PLUGIN_FILE_TEMPLATE_PROVIDER_H
