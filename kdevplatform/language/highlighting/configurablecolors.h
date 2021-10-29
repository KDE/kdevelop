/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CONFIGURABLECOLORS_H
#define KDEVPLATFORM_CONFIGURABLECOLORS_H

#include <language/languageexport.h>

#include <KTextEditor/Attribute>

namespace KDevelop {
class ColorCache;

class KDEVPLATFORMLANGUAGE_EXPORT ConfigurableHighlightingColors
{
public:
    explicit ConfigurableHighlightingColors();

    void addAttribute(int number, const KTextEditor::Attribute::Ptr& attribute);

    KTextEditor::Attribute::Ptr attribute(int number) const;

    void setDefaultAttribute(const KTextEditor::Attribute::Ptr& defaultAttrib);

    KTextEditor::Attribute::Ptr defaultAttribute() const;

private:
    KTextEditor::Attribute::Ptr m_defaultAttribute;
    QHash<int, KTextEditor::Attribute::Ptr> m_attributes;
    QString m_highlightingName;
};

// ######### start CodeHighlightingColors

class CodeHighlightingColors
    : public ConfigurableHighlightingColors
{
public:
    explicit CodeHighlightingColors(ColorCache* cache);
};
}

#endif // KDEVPLATFORM_CONFIGURABLECOLORS_H
