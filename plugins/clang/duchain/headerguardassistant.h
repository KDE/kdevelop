/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef HEADERGUARDASSISTANT_H
#define HEADERGUARDASSISTANT_H

#include <serialization/indexedstring.h>
#include <interfaces/iassistant.h>

#include <clang-c/Index.h>

class HeaderGuardAssistant
    : public KDevelop::IAssistant
{
    Q_OBJECT

public:
    HeaderGuardAssistant(const CXTranslationUnit unit, const CXFile file);
    virtual ~HeaderGuardAssistant() override = default;

    QString title() const override;

    void createActions() override;

private:
    const int m_line;
    const KDevelop::IndexedString m_path;
};

#endif // HEADERGUARDASSISTANT_H
