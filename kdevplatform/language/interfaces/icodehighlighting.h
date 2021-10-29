/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ICODEHIGHLIGHTING_H
#define KDEVPLATFORM_ICODEHIGHLIGHTING_H

#include <language/languageexport.h>

namespace KDevelop {
class ReferencedTopDUContext;
class Declaration;
class DUContext;
class Use;
class IndexedString;

/**
   @short KDevelop text highlighting support interface
   The base class for language support text highlighting
   routines. Languages can apply highlighting to parsed code here.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ICodeHighlighting
{
public:
    virtual ~ICodeHighlighting() {}

    virtual void highlightDUChain(ReferencedTopDUContext context) = 0;
    virtual bool hasHighlighting(IndexedString url) const = 0;
};
}

Q_DECLARE_INTERFACE(KDevelop::ICodeHighlighting, "org.kdevelop.ICodeHighlighting")

#endif // KDEVPLATFORM_ICODEHIGHLIGHTING_H
