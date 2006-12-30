
/***************************************************************************
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SIMPLETYPE_H__
#define __SIMPLETYPE_H__

#include <set>

#include "bithelpers.h"
#include "stringhelpers.h"
#include "completiondebug.h"
#include "typedesc.h"
#include "declarationinfo.h"
#include <qpair.h>

#include "cpp_tags.h"
#include "codemodel.h"

#define NOBACKTRACE

extern QString globalCurrentFile;

using namespace CompletionDebug;
using namespace BitHelpers;
using namespace StringHelpers;

class SimpleTypeImpl;
class SimpleTypeNamespace;
class SimpleTypeFunctionInterface;

typedef KSharedPtr<SimpleTypeImpl> TypePointer;

///if this is set, imported items will get their parent set to the node they were acquired through(which may increase the overall count of items, decrease the caching-performance, and may create other problems in locating stuff)
//#define PHYSICAL_IMPORT

enum Repository {
  CodeModel,
  Catalog,
  StringList,
  Both,
  Undefined
};

class BuiltinTypes {
  public:
    ///This should be used for checking whether a type is builtin
    static bool isBuiltin( const TypeDesc& desc );
    ///If it is a builtin type this returns a short description of the type
    static QString comment( const TypeDesc& desc );
    BuiltinTypes();
  private:
    static QMap<QString, QString> m_types;
};


/**
    A Type can be invalid( hasNode() returns false ) but still have a desc. In that case, the desc is the
    nearest point reached in the resolution of the type.
 
    Warning: The types are not copied when they are assigned to each other,
    they manage internal references, so just changing a type may cause bugs.
    Use makePrivate() before changing. */

class SimpleType {
  public:

    friend class SimpleTypeImpl;

    SimpleType( const SimpleType& rhs ) {
      *this = rhs;
    }

    SimpleType( const QStringList& scope, const HashedStringSet& files, Repository rep = Undefined ) : m_resolved( false ) {
      init( scope, files, rep );
    }

    SimpleType( const QString& text,const HashedStringSet& files, Repository rep = Undefined ) : m_resolved( false ) {
      init( splitType( text ), files, rep );
    };

    SimpleType( Repository rep = Undefined ) : m_resolved( false ) {
      init( QStringList(), HashedStringSet(), rep );
    };

    SimpleType( SimpleTypeImpl* ip ) : m_type( TypePointer( ip ) ), m_resolved( true ) {}

    SimpleType( ItemDom item );

    bool operator < ( SimpleType& rhs );

    /*SimpleType( Tag tag );*/

    SimpleTypeImpl* operator -> () const {
      resolve();
      return &( *m_type );
    }

    TypePointer get
      () const {
        resolve();
        return m_type;
      }

    SimpleTypeImpl& operator * () const {
      resolve();
      return *m_type;
    }

    SimpleType& operator = ( const SimpleType& rhs ) {
      m_type = rhs.m_type;
      m_resolved = rhs.m_resolved;
      m_includeFiles = rhs.m_includeFiles;
      return *this;
    }

    /** Just compares the scope */
    bool operator == ( const SimpleType& rhs ) const {
      return scope() == rhs.scope();
    }

    void makePrivate();

    operator QString() const {
      return str();
    }

    ///lazily returns the scope
    const QStringList& scope() const;

    const QString str() const;

    ///valid() does not check whether the type was found in some model,
    ///it just checks whether this theoretically represents a type.
    inline operator bool () const {
      return valid();
    }

    bool valid() const {
      return !scope().isEmpty();
    }

	static TypePointer globalNamespace() {
		return m_globalNamespace;
	}

    static void setGlobalNamespace( TypePointer tp ) {
      m_globalNamespace = tp;
    }

    static void resetGlobalNamespace() {
      m_globalNamespace = 0;
    }

    ///Since many cross-references are possible, this function breaks them all so that all SimpleTypeImpls can free themselves.
    static void destroyStore();

  private:

    void init( const QStringList& scope, const HashedStringSet& files, Repository rep );

    void resolve( Repository rep = Undefined ) const ;

    HashedStringSet m_includeFiles;

    mutable TypePointer m_type;
    mutable bool m_resolved;
    static TypePointer m_globalNamespace; ///this is bad, but with the current parser we can't clearly determine the correct global-namespace for each class/file
    typedef std::set
      <SimpleTypeImpl*> TypeStore ;
    static TypeStore m_typeStore; ///This is necessary because TypeDescs ind SimpleTypeImpls can have cross-references, and thereby make themselves unreleasable, so each SimpleTypeImpl is stored in this list and destroyed at once by SimpleTypeConfiguration( it breaks all references )
    static TypeStore m_destroyedStore;

    static void registerType( SimpleTypeImpl* tp ) {
      if ( !tp )
        return ;
      m_typeStore.insert( tp );
    }

    static void unregisterType( SimpleTypeImpl* tp ) {
      TypeStore::iterator it = m_typeStore.find( tp );
      if ( it != m_typeStore.end() )
        m_typeStore.erase( it );
      else
        m_destroyedStore.erase( tp );
    }
};


class SimpleTypeConfiguration {
  bool m_invalid;
  public:
    SimpleTypeConfiguration( QString currentFileName = "" ) : m_invalid( false ) {
      globalCurrentFile = currentFileName;
      dbgState.clearCounter();
    }

    void setGlobalNamespace( TypePointer globalNamespace );

    virtual ~SimpleTypeConfiguration() {
      if( !m_invalid ) {
        SimpleType::resetGlobalNamespace();
        SimpleType::destroyStore();
      }
    }
    void invalidate() {
      m_invalid = true;
    }
};


class SimpleTypeImpl : public KShared {
    /*enum ResolutionFlags {
       NoFlag = 0,
       HadTypedef = 1,
       HadTemplate = 2,
       HadAlias = 3
     };*/

  public:
    typedef KSharedPtr<SimpleTypeImpl> TypePointer;

    SimpleTypeImpl( const QStringList& scope ) : m_resolutionCount( 0 ), m_resolutionFlags( NoFlag ), m_scope( scope ) {
	  setScope( m_scope );
      checkTemplateParams();
      reg();
    }

    SimpleTypeImpl( const TypeDesc& desc ) : m_resolutionCount( 0 ), m_resolutionFlags( NoFlag ), m_desc( desc ) {
      if ( !m_desc.name().isEmpty() )
        m_scope.push_back( m_desc.name() );
      else
        m_scope.push_back( "BAD_NAME" );
      reg();
    }

    SimpleTypeImpl( ) : m_resolutionCount( 0 ), m_resolutionFlags( NoFlag ) {
      reg();
    };

	///Returns the template-specialization-string for classes
	virtual QString specialization() const {
		return QString::null;
	}

    class TemplateParamInfo {
      public:
        struct TemplateParam {
          QString name;
          TypeDesc def;
          TypeDesc value;
          int number;
          TemplateParam() : number( 0 ) {}
        }
        ;

        TemplateParamInfo( ) {}
	    
        bool getParam( TemplateParam& target, QString name ) const;

        bool getParam( TemplateParam& target, int number ) const;

        void removeParam( int number );

        void addParam( const TemplateParam& param );

	    int count() const;
	    
      private:
        QMap<int, TemplateParam> m_paramsByNumber;
        QMap<QString, TemplateParam> m_paramsByName;
    };

    int resolutionCount() {
      return m_resolutionCount;
    }

    bool hasResolutionFlag( ResolutionFlags flag ) {
      return m_resolutionFlags & flag;
    }

    enum Operator {
      ArrowOp,
      StarOp,
      AddrOp,
      IndexOp,
      ParenOp,
      NoOp
  };

    static CppCodeCompletion* data;
    TypePointer m_masterProxy; ///If this is only a slave of a namespace-proxy, this holds the proxy.

    virtual ~SimpleTypeImpl() {
      unreg();
    }

    virtual Repository rep() {
      return StringList;
    }

    ///Returns the function-interface if this is a function, else zero
    SimpleTypeFunctionInterface* asFunction();

    virtual bool isNamespace() const {
      return false;
    };

    virtual QString comment() const {
      return "";
    };


    ///Sets the parent of the given slave to either this class, or the proxy of this class
    virtual void setSlaveParent( SimpleTypeImpl& slave );

    void setMasterProxy( TypePointer t ) {
      m_masterProxy = t;
    }

    TypePointer masterProxy() const {
        return m_masterProxy;
    }

    ///@todo remove this and use getTemplateParamInfo instead
    virtual const LocateResult findTemplateParam( const QString& /*name*/ ) {
		return LocateResult();
    }

    virtual TemplateParamInfo getTemplateParamInfo() {
      return TemplateParamInfo();
    }

    virtual void parseParams( TypeDesc desc );

    virtual void takeTemplateParams( TypeDesc desc );

    ///Returns whether the type is really resolved( corresponds to an item in some model )
    virtual bool hasNode() const {
      return false;
    };

    virtual DeclarationInfo getDeclarationInfo() {
      return DeclarationInfo();
    }

    virtual TypePointer clone() {
      return new SimpleTypeImpl( this );
    }

  private:
    int m_resolutionCount;
    ResolutionFlags m_resolutionFlags;

    SimpleTypeImpl( const SimpleTypeImpl& /*rhs*/ ) : KShared() {}

    SimpleTypeImpl& operator = ( const SimpleTypeImpl& /*rhs*/ ) {
      return * this;
    }


    void addResolutionFlag ( ResolutionFlags flag ) {
      m_resolutionFlags = ( ResolutionFlags ) ( m_resolutionFlags | flag );
    }

    void removeResolutionFlag( ResolutionFlags flag ) {
      m_resolutionFlags = remFlag( m_resolutionFlags, flag );
    }

    void increaseResolutionCount() {
      ++m_resolutionCount;
    }

    void setResolutionCount( int val ) {
      m_resolutionCount = val;
    }
#ifndef NOBACKTRACE
    QString create_bt;
#endif

    inline QString createInfo() const {
#ifndef NOBACKTRACE
      return "\n" + create_bt + "\n";
#endif

      return "";
    }

    void reg() {
#ifndef NOBACKTRACE
      create_bt = kdBacktrace();
#endif

      SimpleType::registerType( this );
    }

    void unreg() {
      SimpleType::unregisterType( this );
    }

  protected:
    virtual void invalidateCache() {}
    ;

    ///Secondary cache also depends on the surrounding
    virtual void invalidateSecondaryCache() {}

    virtual void setSecondaryCacheActive( bool active ) {
      Q_UNUSED( active );
    }

    ///Primary cache is the one associated only with this object. It must only be cleared
    ///when the object pointed to changes
    virtual void invalidatePrimaryCache( bool onlyNegative = false ) {}

    static QString operatorToString( Operator op );

  public:
    /*
    operator QString() const {
        return str();
    }

    inline operator QStringList() const {
        return m_scope;
    }*/

    int functionDepth() const {
      return m_desc.pointerDepth();
    }

    void setFunctionDepth( int pc ) {
      m_desc.setPointerDepth( pc );
    }

    void decreaseFunctionDepth() {
      m_desc.setPointerDepth( m_desc.pointerDepth() - 1 );
    }

    int pointerDepth() const {
      return m_desc.pointerDepth();
    }

    void setPointerDepth( int pc ) {
      m_desc.setPointerDepth( pc );
    }


    ///returns the scope(including own name) as string
    QString str() const {
      if ( m_scope.isEmpty() )
        return "";
      return m_scope.join( "::" );
    }

    ///returns the scope(including own name) as string-list
    inline const QStringList& scope() const {
      return m_scope;
    }

	///Returns the scope including template-specialization
	QStringList specializedScope() const {
		QStringList ts = m_scope;
		if( !ts.isEmpty() ) {
			QString s = ts.back() + specialization();
			ts.pop_back();
			ts.push_back( s );
		}
		return ts;
	}

    ///sets the parent-type(type this one is nested in)
    void setParent( TypePointer parent ) {
      if( parent == m_parent ) return;
      invalidateSecondaryCache();
      if ( &( *parent ) == this ) {
        kdDebug( 9007 ) << "setSlaveParent error\n" << kdBacktrace() << endl;
        return ;
      }

      m_parent = parent;
    }

    ///returns whether the type has template-parameters, or one of the parent-types has template-parameters.
    bool usingTemplates() const {
      return !m_desc.templateParams().isEmpty() || ( m_parent && m_parent->usingTemplates() );
    }

    ///This function should add aliases as well as namespace-imports into the given namespace
    virtual void addAliasesTo( SimpleTypeNamespace* ns ) {
    }

    ///An abstract class for building types lazily
  struct TypeBuildInfo : public KShared {
      TypePointer buildCached() {
        if ( m_cache )
          return m_cache;
        else {
          m_cache = build();
          return m_cache;
        }
      }

	  void setCache( const TypePointer& type ) {
		  m_cache = type;
	  }

      virtual TypePointer build() = 0;

      virtual ~TypeBuildInfo() {}

      TypeBuildInfo() {}
    private:

      TypePointer m_cache;

      TypeBuildInfo& operator =( const TypeBuildInfo& rhs ) {
        Q_UNUSED( rhs );
        return *this;
      }
      TypeBuildInfo( const TypeBuildInfo& rhs ) : KShared() {
        Q_UNUSED( rhs );
      }
    };

    ///A class that stores information about a member of some SimpleType
    class MemberInfo {
        KSharedPtr<TypeBuildInfo> m_build;
      public:

        enum MemberType {
          NotFound = 0,
          Function = 1,
          Variable = 2,
          Typedef = 4,
          Template = 8,
          NestedType = 16,
          Namespace = 32,
          AllTypes = 0xffffffff
      } memberType;

        MemberInfo() {
          memberType = NotFound;
        }

        QString memberTypeToString() {
          switch ( memberType ) {
            case Namespace:
            return "namespace";
            case Function:
            return "function";
            case Variable:
            return "variable";
            case NotFound:
            return "not found";
            case Typedef:
            return "typedef";
            case Template:
            return "template-parameter";
            case NestedType:
            return "nested-type";
            default:
            return "unknown";
          };
        }

        typedef KSharedPtr<SimpleTypeImpl> TypePointer;

        void setBuildInfo( KSharedPtr<TypeBuildInfo> build ) {
          m_build = build;
        }

	    void setBuilt( const TypePointer& item ) {
		    m_built = item;
	    }
	    
        TypePointer build() {
		  if( m_built ) return m_built;
	      if ( !m_build ) {
            return TypePointer();
		  } else {
            m_built = m_build->buildCached();
            return m_built;
          }
        }

        operator bool() const {
          return memberType != NotFound;
        }

        QString name;
        LocateResult type;

        ///This member is only filles for variables, typedefs and template-params!
        DeclarationInfo decl;
    private:
	    TypePointer m_built;
    };


    enum LocateMode {
      Normal = 1,
      ExcludeTemplates = 2,
      ExcludeTypedefs = 4,
      ExcludeBases = 8,
      ExcludeParents = 16,
      ExcludeNestedTypes = 32,
      ExcludeNamespaces = 64,
      ForgetModeUpwards = 128,  ///forgets everything, even NoFail, while passing control to the parent
      LocateBase = 4 + 8 + 32 + 64 + 128,  ///searching in the scope visible while the base-declaration of a class
      NoFail = 256,
      TraceAliases = 512,  ///Stores a copy whenever an alias is applied
      OnlyLocalTemplates = 4 + 8 + 16 + 32 + 64 + 256,
      OnlyTemplates = 4 + 8 + 32 + 256
  };

    ///replaces template-parameters from the given structure with their value-types
    TypeDesc replaceTemplateParams( TypeDesc desc, TemplateParamInfo& paramInfo );
    TypeDesc resolveTemplateParams( LocateResult desc, LocateMode mode = Normal );

	//typedef ::LocateResult LocateResult;


    /**By default templates are included while the resolution, so when the type should be addressed from
    outside of the class, ExcludeTemplates should be set as LocateMode, since templates can not be directly accessed    from the outside.
    The resulting type's template-params may not be completely resolved, but can all be resolved locally by that type*/
  LocateResult locateDecType( TypeDesc desc , LocateMode mode = Normal, int dir = 0 , MemberInfo::MemberType typeMask = bitInvert( addFlag( MemberInfo::Variable, MemberInfo::Function ) ) ) {
      TypeDesc td = desc;
      td.clearInstanceInfo();
      LocateResult r = locateType( td, mode, dir, typeMask );
      r.desc() = resolveTemplateParams( r.desc() );
      r->takeInstanceInfo( desc );
      //  r.desc().setPointerDepth( r.desc().pointerDepth() + td.pointerDepth() );
      return r;
    }

    //protected:

  virtual LocateResult locateType( TypeDesc name , LocateMode mode = Normal, int dir = 0 , MemberInfo::MemberType typeMask = bitInvert( addFlag( MemberInfo::Variable, MemberInfo::Function ) ) ) ;

  public:

    LocateResult getFunctionReturnType( QString functionName, QValueList<LocateResult> params = QValueList<LocateResult>() );

    ///Tries to apply the operator and returns the new type. If it fails, it returns an invalid type.
    virtual LocateResult applyOperator( Operator op , QValueList<LocateResult> params = QValueList<LocateResult>() );


    /** In case of a class, returns all base-types */
    virtual QValueList<LocateResult> getBases();

  virtual QStringList getBaseStrings() {
      return QStringList();
    };

    ///This pair contains the found type, and additionally the member-information that helped finding the type
    struct TypeOfResult {
      LocateResult type;
      DeclarationInfo decl;

      TypeOfResult( LocateResult t = LocateResult(), DeclarationInfo d = DeclarationInfo() ) : type( t ), decl( d ) {}

      TypeDesc* operator -> () {
        return & type.desc();
      }

      operator TypeDesc() {
        return type;
      }

      ///should be removed
      operator SimpleType() {
        if ( type->resolved() ) {
          return SimpleType( type->resolved() );
        } else {
          return SimpleType();
        }
      }

      operator bool() {
        return ( bool ) type;
      }
    };

  public:

    virtual TypeOfResult typeOf( const TypeDesc& name, MemberInfo::MemberType typ = addFlag( MemberInfo::Function, MemberInfo::Variable ) );


    ///From outside this should only be called for members like functions/variables etc. Classes will not have their bases resolved when acquired using this function.
    virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type = ( MemberInfo::MemberType ) 0xffffffff ) {
      Q_UNUSED( name );
      Q_UNUSED( type );
      MemberInfo mem;
      mem.memberType = MemberInfo::NotFound;
      return mem;
    };

	///Should return all specializations of a specific class-name
	virtual QValueList<TypePointer> findSpecializations( const QString& name ) {
		return QValueList<TypePointer>();
	}

    /**TypeDescs and SimpleTypeImpls usually have a cross-reference, which creates a circular dependency so that they are never freed using KShared. This function breaks the loop, and also breaks all other possible dependency-loops. After this function was called, the type still contains its private information, but can not not be used to resolve anything anymore. This function is called automatically while the destruction of SimpleTypeConfiguration */
    virtual void breakReferences();

    ///Returns either itself, or the (namespace-)proxy this type is a slave of.
    TypePointer bigContainer();

    ///Returns the parent, eg. the SimpleType this one is nested in.
    SimpleType parent();

    ///this must be a reference, so the desc can be manipulated in-place from outside
    const TypeDesc& desc();

    TypeDesc& descForEdit();

    ///short version
    QString fullType() const {
      return m_desc.fullName();
    }

    ///Similar to fullTypeResolved, except that it also shows addition information about template-params
    QString describeWithParams() ;

    ///returns all information that is available constantly
    QString describe() const {
      QString description = m_desc.fullName() + " (" + m_scope.join( "::" ) + ")" + createInfo();
      return description;
    }

    ///this completely evaluates everything
    QString fullTypeResolved( int depth = 0 );

    ///this completely evaluates everything
    QString fullTypeResolvedWithScope( int depth = 0 );


    QString fullTypeUnresolvedWithScope();

	virtual QValueList<TypePointer> getMemberClasses( const TypeDesc& name ) {
		return QValueList<TypePointer>();
	}

    ///Returns the include-file-set used for resolving this type
    IncludeFiles getFindIncludeFiles();
	
  private:
    QStringList m_scope;
    TypePointer m_parent;
    IncludeFiles m_findIncludeFiles; //Set of include-files used for finding this object

  protected:
    SimpleTypeImpl( SimpleTypeImpl* rhs ) : m_masterProxy( rhs->m_masterProxy ), m_resolutionCount( rhs->m_resolutionCount ), m_resolutionFlags( rhs->m_resolutionFlags ), m_scope( rhs->m_scope ), m_parent( rhs->m_parent ), m_findIncludeFiles( rhs->m_findIncludeFiles ), m_desc( rhs->m_desc ) {
      reg();
    }
  
    TypeDesc m_desc;  ///descibes the local type(so next() must be null)

    /** Tries to extract template-parameters from the scope, resets the params-list */
    virtual void checkTemplateParams ();

    void setScope( const QStringList& scope );

    ///Searches the item IN the bases
    TypeOfResult searchBases ( const TypeDesc& name );

    ///Used to set the include-files that were used to find this type(needed for lazy evaluation of the base-classes)
    void setFindIncludeFiles( const IncludeFiles& files );

		///Should be called within the parent-namespace/class
	virtual void chooseSpecialization( MemberInfo& member );
	
};


class TypeTrace {
    QValueList<QPair< SimpleTypeImpl::MemberInfo, TypeDesc> > m_trace;
  public:

    QValueList<QPair< SimpleTypeImpl::MemberInfo, TypeDesc> >& trace() {
      return m_trace;
    };

    void prepend( const SimpleTypeImpl::MemberInfo& t, const TypeDesc& tail = TypeDesc() ) {
      m_trace.push_front( QPair< SimpleTypeImpl::MemberInfo, TypeDesc>( t, tail ) );
    }

	void prepend( const TypeTrace& trace, int indent = 0 ) {
/*		if( indent != 0 ) {
		QString ind;
			for( int a = 0; a < indent; a++ ) ind += " ";
			for( QValueList<QPair< SimpleTypeImpl::MemberInfo, TypeDesc> >::const_iterator it = trace.m_trace.end(); it != trace.m_trace.begin(); ) {
				--it;
				QPair<SimpleTypeImpl::MemberInfo, TypeDesc> item = *it;
				item.second.prependDecoration( ind );
				m_trace.push_front( item );
			}
 		} else {*/
 		m_trace = trace.m_trace + m_trace;
/* 		}*/
	}
};


#endif 
// kate: indent-mode csands; tab-width 4;

