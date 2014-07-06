/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_CLASSMEMBERDECLARATIONDATA_H
#define KDEVPLATFORM_CLASSMEMBERDECLARATIONDATA_H

#include "declarationdata.h"
#include <language/languageexport.h>

namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT ClassMemberDeclarationData : public DeclarationData
{
public:
  ClassMemberDeclarationData();
  ClassMemberDeclarationData( const ClassMemberDeclarationData& rhs );

  Declaration::AccessPolicy m_accessPolicy;
  bool m_isStatic: 1;
  bool m_isAuto: 1;
  bool m_isFriend: 1;
  bool m_isRegister: 1;
  bool m_isExtern: 1;
  bool m_isMutable: 1;
  bool m_isFinal: 1;
  bool m_isNative: 1;
  bool m_isSynchronized: 1;
  bool m_isStrictFP: 1;
  bool m_isAbstract: 1;
};

}

#endif

