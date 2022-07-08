/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CONFIGURABLECOLORS_H
#define KDEVPLATFORM_CONFIGURABLECOLORS_H

#include "codehighlighting.h"

#include <language/languageexport.h>

#include <KTextEditor/Attribute>

namespace KTextEditor
{
class View;
}

namespace KDevelop {
class ColorCache;

class KDEVPLATFORMLANGUAGE_EXPORT ConfigurableHighlightingColors
{
public:
    explicit ConfigurableHighlightingColors();
    ~ConfigurableHighlightingColors();

    void reset(ColorCache* cache, KTextEditor::View* view);

    KTextEditor::Attribute::Ptr attribute(CodeHighlightingType type) const;

private:
    QHash<CodeHighlightingType, KTextEditor::Attribute::Ptr> m_attributes;
};
}

#endif // KDEVPLATFORM_CONFIGURABLECOLORS_H
