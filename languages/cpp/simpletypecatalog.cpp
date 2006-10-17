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
#include "simpletypecatalog.h"
#include "simpletypefunction.h"
#include "safetycounter.h"

extern SafetyCounter safetyCounter;
extern CppCodeCompletion* cppCompletionInstance;

//SimpleTypeCatalog implementation

TypePointer SimpleTypeCatalog::clone() {
 return new SimpleTypeCachedCatalog( this );
}

QString SimpleTypeCatalog::specialization() const {
 return m_tag.getSpecializationDeclaration();
}

void SimpleTypeCatalog::addAliasesTo( SimpleTypeNamespace* ns ) {
 if ( m_tag.kind() != Tag::Kind_Namespace ) return;
 QValueList<Catalog::QueryArgument> args;

///Insert all namespace-imports
 args << Catalog::QueryArgument( "scope", specializedScope() );
 args << Catalog::QueryArgument( "kind", Tag::Kind_UsingDirective );

 QValueList<Tag> tags( cppCompletionInstance->m_repository->query( args ) );

 for ( QValueList<Tag>::iterator it = tags.begin(); it != tags.end(); ++it ) {
  ns->addAliasMap( "", (*it).name(), HashedString( (*it).fileName() ), true, false );
 }
///Insert all namespace-aliases
 args.clear();
 args << Catalog::QueryArgument( "scope", specializedScope() );
 args << Catalog::QueryArgument( "kind", Tag::Kind_NamespaceAlias );

 tags = cppCompletionInstance->m_repository->query( args );

 for ( QValueList<Tag>::iterator it = tags.begin(); it != tags.end(); ++it ) {
  QVariant v = (*it).attribute( "alias" );
  if ( v.type() == QVariant::String )
   ns->addAliasMap( (*it).name(), v.asString(), HashedString( (*it).fileName() ), true, true );
  else
   kdDebug( 9007 ) << "namespace-alias has no alias-text" << endl;
 }
}

QValueList<TypePointer> SimpleTypeCatalog::getMemberClasses( const TypeDesc& name ) {
 QValueList<TypePointer> ret;

 QValueList<Catalog::QueryArgument> args;

 args << Catalog::QueryArgument( "scope", specializedScope() );
 args << Catalog::QueryArgument( "name", name.name() );

 QValueList<Tag> tags( cppCompletionInstance->m_repository->query( args ) );

 for ( QValueList<Tag>::iterator it = tags.begin(); it != tags.end(); ++it ) {
  if ( (*it).kind() == Tag::Kind_Class ) {
   ///It would be better to return all matched class-names from within findMember and use them from there so all this will be cached too.
   CatalogBuildInfo b( *it, name, TypePointer( this ) );
   TypePointer t = b.buildCached();
   if ( t ) ret << t;
  }
 }

 return ret;
}

SimpleTypeImpl::MemberInfo SimpleTypeCatalog::findMember( TypeDesc name, SimpleTypeImpl::MemberInfo::MemberType type ) {
 MemberInfo ret;
 ret.name = name.name();
 ret.memberType = MemberInfo::NotFound;
 if ( !name ) return ret;

 if (  ( type & MemberInfo::Template) ) {
  LocateResult s = findTemplateParam( name.name() );
  if ( s ) {
   ret.memberType = MemberInfo::Template;
   ret.type = s;
   ret.decl.name = name.name();
   ret.decl.file = m_tag.fileName();
   m_tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
   m_tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  }
 }

 QValueList<Catalog::QueryArgument> args;

 args << Catalog::QueryArgument( "scope", specializedScope() );
 args << Catalog::QueryArgument( "name", name.name() );

 QValueList<Tag> tags( cppCompletionInstance->m_repository->query( args ) );

 if ( tags.isEmpty() )  return ret;

///skip all using-directives
 QValueList<Tag>::iterator it = tags.begin();
 while ( ( (*it).kind() == Tag::Kind_UsingDirective || (*it).kind() == Tag::Kind_NamespaceAlias ) && it != tags.end() )
  ++it;

 if ( it == tags.end() ) return ret;

 Tag tag = *it;

 if ( tag.kind() == Tag::Kind_Variable && (type & MemberInfo::Variable) ) {
  ret.memberType = MemberInfo::Variable;
  ret.type = tagType( tag );
  ret.decl.name = tag.name();
  ret.decl.comment = tag.comment();
  tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
  tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  ret.decl.file = tag.fileName();
 }
 if ( tag.kind() == Tag::Kind_Enumerator && (type & MemberInfo::Variable) ) {
  ret.memberType = MemberInfo::Variable;
  if ( !tag.hasAttribute( "enum" ) ) {
   ret.type = TypeDesc( "const int" );
  } else {
   ret.type = tag.attribute( "enum" ).asString();
   if ( ret.type->name().isEmpty() )
    ret.type = TypeDesc( "const int" );
  }
  ret.decl.name = tag.name();
  ret.decl.comment = tag.comment();
  tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
  tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  ret.decl.file = tag.fileName();
 } else if ( tag.kind() == Tag::Kind_Class && ( type & MemberInfo::NestedType ) ) {
  //if( tag.hasSpecializationDeclaration() ) {
  //Choose another tag(the main class, not a specialization).
  bool hasSpecializationDeclaration = tag.hasSpecializationDeclaration();
  bool isIncluded = name.includeFiles()[tag.fileName()];
  if ( hasSpecializationDeclaration || !isIncluded ) {
   for ( QValueList<Tag>::const_iterator it = tags.begin(); it != tags.end(); ++it ) {
    if ( (*it).kind() == Tag::Kind_Class && !(*it).hasSpecializationDeclaration() ) {
     if ( name.includeFiles()[(*it).fileName()] ) {
      tag = *it;
      isIncluded = true;
      hasSpecializationDeclaration = false;
     } else if ( hasSpecializationDeclaration ) {
      tag = *it;
      hasSpecializationDeclaration = false;
      isIncluded = false;
     }

     if ( isIncluded && !hasSpecializationDeclaration ) break;
    }
   }
  }
  //only accept non-specialized classes
  if ( !tag.hasSpecializationDeclaration() ) {
   ret.setBuildInfo( new CatalogBuildInfo( tag, name, TypePointer( this ) ) );
   ret.memberType = MemberInfo::NestedType;
   ret.type = name;
  }
 } else if ( tag.kind() == Tag::Kind_Typedef && ( type & MemberInfo::Typedef ) ) {
  ret.memberType = MemberInfo::Typedef;
  ret.type = tagType( tag );
  ret.decl.name = tag.name();
  ret.decl.comment = tag.comment();
  tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
  tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  ret.decl.file = tag.fileName();
 } else if ( tag.kind() == Tag::Kind_Enum && ( type & MemberInfo::Typedef ) ) {
  ret.memberType = MemberInfo::Typedef;
  ret.type = TypeDesc( "const int" );
  ret.decl.name = tag.name();
  ret.decl.comment = tag.comment();
  tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
  tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  ret.decl.file = tag.fileName();
 } else if ( (tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function)  && ( type & MemberInfo::Function ) ) {
  ret.memberType = MemberInfo::Function;
  ret.type = tagType( tag );
  ret.type->increaseFunctionDepth();
  ret.setBuildInfo( new SimpleTypeCatalogFunction::CatalogFunctionBuildInfo( tags, name, TypePointer( this ) ) );
 } else if ( tag.kind() == Tag::Kind_Namespace && ( type & MemberInfo::Namespace ) ) {
  ret.setBuildInfo( new CatalogBuildInfo( tag , name, TypePointer( this ) ) );
  ret.memberType = MemberInfo::Namespace;
  ret.type = name;
 }

///Check if it is a template-name

//if( !ret.type) ret.memberType = MemberInfo::NotFound; //constructor..

 chooseSpecialization( ret );
 return ret;
}

Tag SimpleTypeCatalog::findSubTag( const QString& name ) {
 if ( name.isEmpty() ) return Tag();

 QValueList<Catalog::QueryArgument> args;
 QTime t;

 t.start();
 args << Catalog::QueryArgument( "scope", specializedScope() );
 args << Catalog::QueryArgument( "name", name );

 QValueList<Tag> tags( cppCompletionInstance->m_repository->query( args ) );
 if ( ! tags.isEmpty() ) {
  //ifVerbose( dbg() << "findTag: \"" << str() << "\": tag \"" << name << "\" found " << endl );
  return tags.front();
 } else {
  //ifVerbose( dbg() << "findTag: \"" << str() << "\": tag \"" << name << "\" not found " << endl );
  return Tag();
 }
}

QValueList<Tag> SimpleTypeCatalog::getBaseClassList( ) {
 if ( scope().isEmpty() )
  return QValueList<Tag>();
 return cppCompletionInstance->m_repository->getBaseClassList( scope().join("::") + specialization() );
}

void SimpleTypeCatalog::initFromTag() {
 QStringList l = m_tag.scope();
 l << m_tag.name();
 setScope( l );
}

void SimpleTypeCatalog::init() {
 if ( !scope().isEmpty() ) {
  QStringList l = scope();
  QStringList cp = l;
  cp.pop_back();
  setScope( cp );
  m_tag = findSubTag( l.back() );
  setScope( l );
  //initFromTag( ); ///must not be done, because it may initialize to wrong namespaces etc.
 }
}

DeclarationInfo SimpleTypeCatalog::getDeclarationInfo() {
 DeclarationInfo ret;

 ret.name = fullTypeResolved();
 if ( m_tag ) {
  ret.file = m_tag.fileName();
  m_tag.getStartPosition( &ret.startLine, &ret.startCol );
  m_tag.getEndPosition( &ret.endLine, &ret.endCol );
  ret.comment = m_tag.comment();
 }

 return ret;
}

QValueList<LocateResult> SimpleTypeCatalog::getBases() {
 Debug d( "#getbases#" );
 if ( !d || !safetyCounter ) {
  //ifVerbose( dbg() << "\"" << str() << "\": recursion to deep while getting bases" << endl );
  return QValueList<LocateResult>();
 }

 QValueList<LocateResult> ret;
// try with parentsc
 QTime t;
 t.restart();
 QValueList<Tag> parents( getBaseClassList() );

 QValueList<Tag>::Iterator it = parents.begin();
 while ( it != parents.end() ) {
  Tag & tag = *it;
  ++it;

  CppBaseClass<Tag> info( tag );

  ret << locateDecType( info.baseClass() , LocateBase );
 }
 return ret;
}

SimpleTypeImpl::TemplateParamInfo SimpleTypeCatalog::getTemplateParamInfo() {
 TemplateParamInfo ret;

 if ( m_tag ) {
  if ( m_tag.hasAttribute( "tpl" ) ) {
   QStringList l = m_tag.attribute( "tpl" ).asStringList();

   TypeDesc::TemplateParams templateParams = m_desc.templateParams();
   uint pi = 0;
   QStringList::const_iterator it = l.begin();
   while ( it != l.end() ) {
    TemplateParamInfo::TemplateParam curr;
    curr.name = *it;
    curr.number = pi;
    ++pi;
    ++it;
    if ( it != l.end() ) {
     curr.def = *it;
     ++it;
    }
    if ( pi < templateParams.count() )
     curr.value = *templateParams[pi];
    ret.addParam( curr );
   };
  }
 }

 return ret;
}

const LocateResult SimpleTypeCatalog::findTemplateParam( const QString& name ) {
 if ( m_tag ) {
  if ( m_tag.hasAttribute( "tpl" ) ) {
   QStringList l = m_tag.attribute( "tpl" ).asStringList();
   ///we need the index, so count the items through
   uint pi = 0;

   QStringList::const_iterator it = l.begin();
   while ( it != l.end() && *it != name ) {
    ++pi;
    ++it;
    if ( it != l.end() ) ++it;
   };

   TypeDesc::TemplateParams templateParams = m_desc.templateParams();

   if ( it != l.end() &&  pi < templateParams.count() ) {
    return *templateParams[pi];
   } else {
    if ( it != l.end() && *it == name && !(*it).isEmpty()) {
     ++it;
     if ( it != l.end() && !(*it).isEmpty() ) {
      ifVerbose( dbg() << "using default-template-type " << *it << " for " << name << endl );
      return TypeDesc( *it );     ///return default-parameter
     }
    }
   }
  }
 }
 return LocateResult();
};

//SimpleTypeCatalog::CatalogBuildInfo implementation
TypePointer SimpleTypeCatalog::CatalogBuildInfo::build() {
 if ( !m_tag )
  return TypePointer();
 else {
  TypePointer tp = new SimpleTypeCachedCatalog( m_tag );
  tp->parseParams( m_desc );
  if ( m_parent ) tp->setParent( m_parent->bigContainer() );
  return tp;
 }

}

// kate: indent-mode csands; tab-width 4;

