/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include <QtCore/QSet>
#include <QtCore/QList>

#include "typepointer.h"
#include "language/duchain/identifier.h"

namespace KDevelop
{
class AbstractTypeDataRequest;

class AbstractType;
class IntegralType;
class PointerType;
class ReferenceType;
class FunctionType;
class StructureType;
class ArrayType;

class AbstractTypeData;
class IntegralTypeData;
class PointerTypeData;
class ReferenceTypeData;
class FunctionTypeData;
class StructureTypeData;
class ArrayTypeData;
class DelayedTypeData;

class IndexedString;
class IndexedType;

class TypeExchanger;

#define TYPE_DECLARE_DATA(Class) \
    inline Class##Data* d_func_dynamic() { makeDynamic(); return reinterpret_cast<Class##Data *>(d_ptr); } \
    inline const Class##Data* d_func() const { return reinterpret_cast<const Class##Data *>(d_ptr); }

#define TYPE_D(Class) const Class##Data * const d = d_func()
#define TYPE_D_DYNAMIC(Class) Class##Data * const d = d_func_dynamic()


class KDEVPLATFORMLANGUAGE_EXPORT TypeVisitor
{
public:
  virtual ~TypeVisitor ();

  virtual bool preVisit (const AbstractType *) = 0;
  virtual void postVisit (const AbstractType *) = 0;

  ///Return whether sub-types should be visited(same for the other visit functions)
  virtual bool visit(const AbstractType*) = 0;

  virtual void visit (const IntegralType *) = 0;

  virtual bool visit (const PointerType *) = 0;
  virtual void endVisit (const PointerType *) = 0;

  virtual bool visit (const ReferenceType *) = 0;
  virtual void endVisit (const ReferenceType *) = 0;

  virtual bool visit (const FunctionType *) = 0;
  virtual void endVisit (const FunctionType *) = 0;

  virtual bool visit (const StructureType *) = 0;
  virtual void endVisit (const StructureType *) = 0;

  virtual bool visit (const ArrayType *) = 0;
  virtual void endVisit (const ArrayType *) = 0;
};

class KDEVPLATFORMLANGUAGE_EXPORT SimpleTypeVisitor : public TypeVisitor
{
public:
  ///When using SimpleTypeVisitor, this is the only function you must override to collect all types.
  virtual bool visit(const AbstractType*) = 0;

  virtual bool preVisit (const AbstractType *) ;
  virtual void postVisit (const AbstractType *) ;

  virtual void visit (const IntegralType *) ;

  virtual bool visit (const PointerType *) ;
  virtual void endVisit (const PointerType *) ;

  virtual bool visit (const ReferenceType *) ;
  virtual void endVisit (const ReferenceType *) ;

  virtual bool visit (const FunctionType *) ;
  virtual void endVisit (const FunctionType *) ;

  virtual bool visit (const StructureType *) ;
  virtual void endVisit (const StructureType *) ;

  virtual bool visit (const ArrayType *) ;
  virtual void endVisit (const ArrayType *) ;
};

/**
 * \brief Base class for all types.
 *
 * The AbstractType class is a base class from which all types derive.  It features:
 * - mechanisms for visiting types
 * - toString() feature
 * - equivalence feature
 * - cloning of types, and
 * - hashing and indexing
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractType : public TypeShared
{
public:
  typedef TypePtr<AbstractType> Ptr;

  /// Constructor.
  AbstractType();
  /// Copy Constructor.
  AbstractType(const AbstractType& rhs);
  /// Constructor from data.
  AbstractType(AbstractTypeData& dd);
  /// Destructor.
  virtual ~AbstractType ();

  /**
   * Visitor method.  Called by TypeVisitor to visit the type heirachy.
   *
   * \param v visitor which is calling this function.
   */
  void accept(TypeVisitor *v) const;

  /**
   * Convenience visitor method which can be called with a null type.
   *
   * \param type type to visit, may be null.
   * \param v visitor which is visiting the given \a type
   */
  static void acceptType(AbstractType::Ptr type, TypeVisitor *v);

  /**
   * Returns this type as a string, preferably the same as it is expressed in the code.
   *
   * \return this type as a string
   */
  virtual QString toString() const = 0;

  ///Must always be called before anything in the data pointer is changed!
  ///If it's not called beforehand, the type-repository gets corrupted
  void makeDynamic();

  ///Should return whether this type's content equals the given one
  ///Since this is used by the type-repository, it must compare ALL members of the data type.
  virtual bool equals(const AbstractType* rhs) const = 0;

  /**
   * Should create a clone of the source-type, with as much data copied as possible without breaking the du-chain.
   * */
  virtual AbstractType* clone() const = 0;

  /**
   * A hash-value that should have the following properties:
   * - When two types match on equals(), it should be same.
   * - When two types don't match on equals(), it should be different with a high probability.
   * */
  virtual uint hash() const = 0;

  ///This can also be called on zero types, those can then be reconstructed from the zero index
  IndexedType indexed() const;

  /// Enumeration of major data types.
  enum WhichType {
    TypeAbstract  /**< an abstract type */,
    TypeIntegral  /**< an integral */,
    TypePointer   /**< a pointer*/,
    TypeReference /**< a reference */,
    TypeFunction  /**< a function */,
    TypeStructure /**< a structure */,
    TypeArray     /**< an array */,
    TypeDelayed   /**< a delayed type */,
    TypeForward   /**< a foward declaration type */
  };

  /**
   * Determine which data type this abstract type represents.
   *
   * \returns the data type represented by this type.
   */
  virtual WhichType whichType() const;

  enum {
    Identity = 1
  };

  /**
   * Should, like accept0, be implemented by all types that hold references to other types.
   *
   * \todo document function
   * */
  virtual void exchangeTypes( TypeExchanger* exchanger );

  /**
   * Method to create copies of internal type data. You must use this to create the internal
   * data instances in copy constructors. It is needed, because it may need to allocate more memory
   * for appended lists.
   *
   * \param rhs data to copy
   * \returns copy of the data
   */
  template<class DataType>
  static DataType& copyData(const DataType& rhs) {
    size_t size;
    if(!rhs.m_dynamic)
      size = sizeof(DataType); //Create a dynamic data instance
    else
      size = rhs.dynamicSize(); //Create a constant data instance, that holds all the data embedded.

    return *new (new char[size]) DataType(rhs);
  }

  /**
   * Method to create internal data structures. Use this in normal constructors.
   *
   * \returns the internal data structure
   */
  template<class DataType>
  static DataType& createData() {
    return *new (new char[sizeof(DataType)]) DataType();
  }

  typedef AbstractTypeData Data;

protected:
  /**
   * Visitor method, reimplement to allow visiting of types.
   *
   * \param v visitor which is visiting.
   */
  virtual void accept0 (TypeVisitor *v) const = 0;
  AbstractTypeData* d_ptr;

  TYPE_DECLARE_DATA(AbstractType)

  friend class AbstractTypeDataRequest;
};

/**
 * A class that can be used to walk through all types that are references from one type, and exchange them with other types.
 * Examples for such types: Base-classes of a class, function-argument types of a function, etc.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT TypeExchanger {
  public:
    virtual ~TypeExchanger() {
    }

    /**
     * By default should return the given type, and can return another type that the given should be replaced with.
     * Types should allow replacing all their held types using this from within their exchangeTypes function.
     * */
    virtual AbstractType::Ptr exchange( const AbstractType::Ptr& ) = 0;
    /**
     * Should member-types be exchanged?(Like the types of a structure's members) If false, only types involved in the identity will be exchanged.
     * */
    virtual bool exchangeMembers() const = 0;
};

/**
 * \short Indexed type pointer.
 *
 * IndexedType is a class which references a type by an index.
 * This way the type can be stored to disk.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedType {
  public:
    /// Constructor.
    IndexedType(uint index = 0) : m_index(index) {
    }

    /**
     * Access the type.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    AbstractType::Ptr type() const;

    /// Determine if the type is valid. \returns true if valid, otherwise false.
    bool isValid() const {
      return (bool)m_index;
    }

    /// \copydoc
    operator bool() const {
      return (bool)m_index;
    }

    /// Equivalence operator. \param rhs indexed type to compare. \returns true if equal (or both invalid), otherwise false.
    bool operator==(const IndexedType& rhs) const {
      return m_index == rhs.m_index;
    }

    /// Not equal operator. \param rhs indexed type to compare. \returns true if types are not the same, otherwise false.
    bool operator!=(const IndexedType& rhs) const {
      return m_index != rhs.m_index;
    }

    /// Access the type's hash value. \returns the hash value.
    uint hash() const {
      return m_index;
    }

    /// Access the type's index. \returns the index.
    uint index() const {
      return m_index;
    }

  private:
    uint m_index;
};

/**
 * \short A type representing inbuilt data types.
 *
 * IntegralType is used to represent types which are native to a programming languge,
 * such as (e.g.) int, float, double, char, bool etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IntegralType: public AbstractType
{
public:
  typedef TypePtr<IntegralType> Ptr;

  /// Default constructor
  IntegralType();
  /// Copy constructor. \param rhs type to copy
  IntegralType(const IntegralType& rhs);
  /// Constructor for named integral types. \param name name of this type
  IntegralType(const IndexedString& name);
  /// Constructor using raw data. \param data internal data.
  IntegralType(IntegralTypeData& data);
  /// Destructor
  virtual ~IntegralType();

  /// Access the name of this type. \returns the type's name.
  const IndexedString& name() const;

  /// Set the name of this type.  \param name the type's name.
  void setName(const IndexedString& name);

  /// Equivalence operator. \param other other integral type to compare. \returns true if types are equal, otherwise false
  bool operator == (const IntegralType &other) const;

  /// Not equal operator. \param other other integral type to compare. \returns false if types are equal, otherwise false
  bool operator != (const IntegralType &other) const;

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  enum {
    Identity = 2
  };

  typedef IntegralTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(IntegralType)
};

/**
 * \short A type representing pointer types.
 *
 * PointerType is used to represent types which hold a pointer to a location
 * in memory.
 */
class KDEVPLATFORMLANGUAGE_EXPORT PointerType: public AbstractType
{
public:
  typedef TypePtr<PointerType> Ptr;

  /// Default constructor
  PointerType ();
  /// Copy constructor. \param rhs type to copy
  PointerType(const PointerType& rhs);
  /// Constructor using raw data. \param data internal data.
  PointerType(PointerTypeData& data);
  /// Destructor
  virtual ~PointerType();

  /// Equivalence operator. \param other other pointer type to compare. \returns true if types are equal, otherwise false
  bool operator != (const PointerType &other) const;
  /// Not equal operator. \param other other pointer type to compare. \returns false if types are equal, otherwise false
  bool operator == (const PointerType &other) const;

  /**
   * Sets the base type of the pointer, ie. what type of data the pointer points to.
   *
   * \param baseType the base type.
   */
  void setBaseType(AbstractType::Ptr type);

  /**
   * Retrieve the base type of the pointer, ie. what type of data the pointer points to.
   *
   * \returns the base type.
   */
  AbstractType::Ptr baseType () const;

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  virtual void exchangeTypes( TypeExchanger* exchanger );

  enum {
    Identity = 3
  };

  typedef PointerTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(PointerType)
};

/**
 * \short A type representing reference types.
 *
 * ReferenceType is used to represent types which hold a reference to a
 * variable.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ReferenceType: public AbstractType
{
public:
  typedef TypePtr<ReferenceType> Ptr;

  /// Default constructor
  ReferenceType ();
  /// Copy constructor. \param rhs type to copy
  ReferenceType (const ReferenceType& rhs);
  /// Constructor using raw data. \param data internal data.
  ReferenceType(ReferenceTypeData& data);
  /// Destructor
  virtual ~ReferenceType();

  /**
   * Retrieve the referenced type, ie. what type of data this type references.
   *
   * \returns the base type.
   */
  AbstractType::Ptr baseType () const;

  /**
   * Sets the referenced type, ie. what type of data this type references.
   *
   * \param baseType the base type.
   */
  void setBaseType(AbstractType::Ptr baseType);

  /// Equivalence operator. \param other other reference type to compare. \returns true if types are equal, otherwise false
  bool operator == (const ReferenceType &other) const;

  /// Not equal operator. \param other other reference type to compare. \returns false if types are equal, otherwise false
  bool operator != (const ReferenceType &other) const;

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  virtual void exchangeTypes( TypeExchanger* exchanger );

  enum {
    Identity = 4
  };

  typedef ReferenceTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(ReferenceType)
};

/**
 * \short A type representing function types.
 *
 * A FunctionType is represents the type of a function.  It provides access
 * to the return type, and number and types of the arguments.
 */
class KDEVPLATFORMLANGUAGE_EXPORT FunctionType : public AbstractType
{
public:
  typedef TypePtr<FunctionType> Ptr;

  /// An enumeration of sections of the function signature that can be returned.
  enum SignaturePart {
    SignatureWhole /**< When this is given to toString(..), a string link "RETURNTYPE (ARGTYPE1, ARGTYPE1, ..)" is returned */,
    SignatureReturn /**< When this is given, only a string that represents the return-type is returned */,
    SignatureArguments /**< When this is given, a string that represents the arguments like "(ARGTYPE1, ARGTYPE1, ..)" is returend */
  };

  /// Default constructor
  FunctionType();
  /// Copy constructor. \param rhs type to copy
  FunctionType(const FunctionType& rhs);
  /// Constructor using raw data. \param data internal data.
  FunctionType(FunctionTypeData& data);
  /// Destructor
  ~FunctionType();

  /**
   * Retrieve the return type of the function.
   *
   * \returns the return type.
   */
  AbstractType::Ptr returnType () const;

  /**
   * Sets the return type of the function.
   *
   * \param returnType the return type.
   */
  void setReturnType(AbstractType::Ptr returnType);

  /**
   * Retrieve the list of types of the function's arguments.
   *
   * \returns the argument types.
   */
  QList<AbstractType::Ptr> arguments () const;

  /**
   * Add an argument to the function, specifying what type it takes.
   *
   * \param argument the argument's type
   */
  void addArgument(AbstractType::Ptr argument);

  /**
   * Remove an argument type from the function.
   *
   * \param argument the argument type to remove
   * \todo this function doesn't seem to be used, remove it?
   */
  void removeArgument(AbstractType::Ptr argument);

  /// Equivalence operator. \param other other function type to compare. \returns true if types are equal, otherwise false
  bool operator == (const FunctionType &other) const;

  /// Not equal operator. \param other other function type to compare. \returns false if types are equal, otherwise false
  bool operator != (const FunctionType &other) const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  virtual QString toString() const;

  /**
   * This function creates a string that represents the requested part of
   * this function's signature.
   *
   * \param sigPart part of the signature requested.
   * \returns the signature as text.
   */
  virtual QString partToString( SignaturePart sigPart ) const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual void exchangeTypes( TypeExchanger* exchanger );

  enum {
    Identity = 5
  };

  typedef FunctionTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(FunctionType)
};

/**
 * \short A type representing structure types.
 *
 * StructureType represents all structures, including classes,
 * interfaces, etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT StructureType : public AbstractType
{
public:
  typedef TypePtr<StructureType> Ptr;

  /// Default constructor
  StructureType();
  /// Copy constructor. \param rhs type to copy
  StructureType(const StructureType& rhs);
  /// Constructor using raw data. \param data internal data.
  StructureType(StructureTypeData& data);
  /// Destructor
  virtual ~StructureType();

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  enum {
    Identity = 6
  };

  typedef StructureTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(StructureType)
};

class KDEVPLATFORMLANGUAGE_EXPORT ArrayType : public AbstractType
{
public:
  typedef TypePtr<ArrayType> Ptr;

  /// Default constructor
  ArrayType();
  /// Copy constructor. \param rhs type to copy
  ArrayType(const ArrayType& rhs);
  /// Constructor using raw data. \param data internal data.
  ArrayType(ArrayTypeData& data);
  /// Destructor
  virtual ~ArrayType();

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  /**
   * Retrieve the dimension of this array type. Multiple-dimensioned
   * arrays will have another array type as their elementType().
   *
   * \returns the dimension of the array, or zero if the array is dimensionless (eg. int[])
   */
  int dimension () const;

  /**
   * Set this array type's dimension.
   *
   * \param dimension new dimension, set to zero for a dimensionless type (eg. int[])
   */
  void setDimension(int dimension);

  /**
   * Retrieve the element type of the array, e.g. "int" for int[3].
   *
   * \returns the element type.
   */
  AbstractType::Ptr elementType () const;

  /**
   * Set the element type of the array, e.g. "int" for int[3].
   *
   * \returns the element type.
   */
  void setElementType(AbstractType::Ptr type);

  bool operator == (const ArrayType &other) const;

  bool operator != (const ArrayType &other) const;

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual void exchangeTypes( TypeExchanger* exchanger );

  enum {
    Identity = 7
  };

  typedef ArrayTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(ArrayType)
};

/**
 * \short A type which has not yet been resolved.
 *
 * Delayed types can be used for any types that cannot be resolved in the moment they are encountered.
 * They can be used for example in template-classes, or to store the names of unresolved types.
 * In a template-class, many types can not be evaluated at the time they are used, because they depend on unknown template-parameters.
 * Delayed types store the way the type would be searched, and can be used to find the type once the template-paremeters have values.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT DelayedType : public KDevelop::AbstractType
{
public:
  typedef TypePtr<DelayedType> Ptr;

  enum Kind {
    Delayed /**< The type should be resolved later. This is the default. */,
    Unresolved /**< The type could not be resolved */
  };

  DelayedType();
  DelayedType(const DelayedType& rhs);
  DelayedType(DelayedTypeData& data);
  virtual ~DelayedType();

  KDevelop::TypeIdentifier identifier() const;
  void setIdentifier(const KDevelop::TypeIdentifier& identifier);

  virtual QString toString() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  Kind kind() const;
  void setKind(Kind kind);

  virtual uint hash() const;

  virtual WhichType whichType() const;

  enum {
    Identity = 8
  };

  typedef DelayedTypeData Data;

  protected:
    virtual void accept0 (KDevelop::TypeVisitor *v) const ;
    TYPE_DECLARE_DATA(DelayedType)
};

template <class T>
uint qHash(const TypePtr<T>& type) { return (uint)((size_t)type.unsafeData()); }


/**
 * You can use these instead of dynamic_cast, for basic types it has better performance because it checks the whichType() member
*/

template<class To>
inline To fastCast(AbstractType* from) {
  return dynamic_cast<To>(from);
}

template<class To>
inline const To fastCast(const AbstractType* from) {
  return const_cast<const To>(fastCast<To>(const_cast<AbstractType*>(from))); //Hack so we don't need to define the functions twice, once for const, and once for not const
}

template<>
inline ReferenceType* fastCast<ReferenceType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeReference)
    return 0;
  else
    return static_cast<ReferenceType*>(from);
}

template<>
inline PointerType* fastCast<PointerType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypePointer)
    return 0;
  else
    return static_cast<PointerType*>(from);
}

template<>
inline IntegralType* fastCast<IntegralType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeIntegral)
    return 0;
  else
    return static_cast<IntegralType*>(from);
}

template<>
inline FunctionType* fastCast<FunctionType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeFunction)
    return 0;
  else
    return static_cast<FunctionType*>(from);
}

template<>
inline StructureType* fastCast<StructureType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeStructure)
    return 0;
  else
    return static_cast<StructureType*>(from);
}

template<>
inline ArrayType* fastCast<ArrayType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeArray)
    return 0;
  else
    return static_cast<ArrayType*>(from);
}

template<>
inline DelayedType* fastCast<DelayedType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeDelayed)
    return 0;
  else
    return static_cast<DelayedType*>(from);
}

}




#endif // TYPESYSTEM_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
