/*
   Copyright (c) 2010 KDAB

   Author: Tobias Koenig <tokoe@kde.org>

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

#ifndef QPROPERTYDECLARATION_H
#define QPROPERTYDECLARATION_H

#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classmemberdeclarationdata.h>
#include "cppduchainexport.h"

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT QPropertyDeclarationData : public KDevelop::ClassMemberDeclarationData
{
public:
  QPropertyDeclarationData() {}
  QPropertyDeclarationData(const QPropertyDeclarationData& rhs)
    : ClassMemberDeclarationData(rhs),
      m_readMethod(rhs.m_readMethod),
      m_writeMethod(rhs.m_writeMethod),
      m_resetMethod(rhs.m_resetMethod),
      m_notifyMethod(rhs.m_notifyMethod),
      m_designableMethod(rhs.m_designableMethod),
      m_scriptableMethod(rhs.m_scriptableMethod),
      m_isDesignable(rhs.m_isDesignable),
      m_isScriptable(rhs.m_isScriptable),
      m_isStored(rhs.m_isStored),
      m_isUser(rhs.m_isUser),
      m_isConstant(rhs.m_isConstant),
      m_isFinal(rhs.m_isFinal)
  {
  }

  KDevelop::IndexedDeclaration m_readMethod;
  KDevelop::IndexedDeclaration m_writeMethod;
  KDevelop::IndexedDeclaration m_resetMethod;
  KDevelop::IndexedDeclaration m_notifyMethod;
  KDevelop::IndexedDeclaration m_designableMethod;
  KDevelop::IndexedDeclaration m_scriptableMethod;

  bool m_isDesignable : 1;
  bool m_isScriptable : 1;
  bool m_isStored : 1;
  bool m_isUser : 1;
  bool m_isConstant : 1;
  bool m_isFinal : 1;
};

/**
 * @short Represents a Q_PROPERTY macro declaration.
 */
class KDEVCPPDUCHAIN_EXPORT QPropertyDeclaration : public KDevelop::ClassMemberDeclaration
{
public:
  QPropertyDeclaration(const QPropertyDeclaration& rhs);
  QPropertyDeclaration(QPropertyDeclarationData& data);
  QPropertyDeclaration(const KDevelop::SimpleRange& range, KDevelop::DUContext* context);

  /**
   * Sets the @p declaration of the READ method of the property.
   */
  void setReadMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the READ method of the property.
   */
  KDevelop::IndexedDeclaration readMethod() const;

  /**
   * Sets the @p declaration of the WRITE method of the property.
   */
  void setWriteMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the WRITE method of the property.
   */
  KDevelop::IndexedDeclaration writeMethod() const;

  /**
   * Sets the @p declaration of the RESET method of the property.
   */
  void setResetMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the RESET method of the property.
   */
  KDevelop::IndexedDeclaration resetMethod() const;

  /**
   * Sets the @p declaration of the NOTIFY signal method of the property.
   */
  void setNotifyMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the NOTIFY signal method of the property.
   */
  KDevelop::IndexedDeclaration notifyMethod() const;

  /**
   * Sets the @p declaration of the DESIGNABLE method of the property.
   */
  void setDesignableMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the DESIGNABLE method of the property.
   */
  KDevelop::IndexedDeclaration designableMethod() const;

  /**
   * Sets the @p declaration of the SCRIPTABLE method of the property.
   */
  void setScriptableMethod(const KDevelop::IndexedDeclaration &declaration);

  /**
   * Returns the declaration of the SCRIPTABLE method of the property.
   */
  KDevelop::IndexedDeclaration scriptableMethod() const;

  /**
   * Sets whether the property is @p designable.
   */
  void setIsDesignable(bool designable);

  /**
   * Returns whether the property is designable.
   */
  bool isDesignable() const;

  /**
   * Sets whether the property is @p scriptable.
   */
  void setIsScriptable(bool scriptable);

  /**
   * Returns whether the property is scriptable.
   */
  bool isScriptable() const;

  /**
   * Sets whether the property value is @p stored.
   */
  void setIsStored(bool stored);

  /**
   * Returns whether the property value is stored.
   */
  bool isStored() const;

  /**
   * Sets whether the property can be changed by the @p user.
   */
  void setIsUser(bool user);

  /**
   * Returns whether the property can be changed by the user.
   */
  bool isUser() const;

  /**
   * Sets whether the property is @p constant.
   */
  void setIsConstant(bool constant);

  /**
   * Returns whether the property is constant.
   */
  bool isConstant() const;

  /**
   * Sets whether the property is @p final.
   */
  void setIsFinal(bool final);

  /**
   * Returns whether the property is final.
   */
  bool isFinal() const;

  enum {
    Identity = 75
  };

private:
  virtual KDevelop::Declaration* clonePrivate () const;
  DUCHAIN_DECLARE_DATA(QPropertyDeclaration)
};

}

#endif // QPROPERTYDECLARATION_H
