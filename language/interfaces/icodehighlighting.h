/* This file is part of the KDE project
   Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_ICODEHIGHLIGHTING_H
#define KDEVPLATFORM_ICODEHIGHLIGHTING_H

#include "languageexport.h"

namespace KDevelop
{

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
