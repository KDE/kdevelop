/*
 * This file is part of KDevelop
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_CONFIGURABLECOLORS_H
#define KDEVPLATFORM_CONFIGURABLECOLORS_H

#include "codehighlighting.h"
#include "colorcache.h"

#include <language/languageexport.h>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT ConfigurableHighlightingColors {
  public:
    ConfigurableHighlightingColors(QString highlightingName);

    void addAttribute(int number, KTextEditor::Attribute::Ptr attribute);

    KTextEditor::Attribute::Ptr getAttribute(int number) const;

    void setDefaultAttribute(KTextEditor::Attribute::Ptr defaultAttrib);

    KTextEditor::Attribute::Ptr defaultAttribute() const;

  private:
    KTextEditor::Attribute::Ptr m_defaultAttribute;
    QHash<int, KTextEditor::Attribute::Ptr> m_attributes;
    QString m_highlightingName;
};

// ######### start CodeHighlightingColors

class CodeHighlightingColors : public ConfigurableHighlightingColors {
  public:
    CodeHighlightingColors(ColorCache* cache);
};
}

#endif // KDEVPLATFORM_CONFIGURABLECOLORS_H

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
