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

#include "simpletypefunction.h"
#include "safetycounter.h"
#include "simpletypenamespace.h"
#include <driver.h>

extern SafetyCounter safetyCounter;
extern CppCodeCompletion* cppCompletionInstance;

HashedStringSet getIncludeFiles( const ItemDom& item ) {
    if( item ) {
        FileDom f =  item->codeModel()->fileByName( item->fileName() );
        if( f ) {
            ParseResultPointer p = f->parseResult();
            if( p ) {
                ParsedFilePointer pp = dynamic_cast<ParsedFile*>( p.data() );
                if( pp ) {
                    return pp->includeFiles();
                }
            }
        }
    }

    return HashedStringSet();
}



//SimpleTypeFunctionInterface implementation
  
QString SimpleTypeFunctionInterface::signature() 
{
  QString sig = "( ";
  SimpleTypeImpl* asType = dynamic_cast<SimpleTypeImpl*>( this );
    
  QStringList argDefaults = getArgumentDefaults();
  QStringList argNames = getArgumentNames();
  QValueList<TypeDesc> argTypes = getArgumentTypes();
  QValueList<LocateResult> argRealTypes;
    
  if( asType ) {
    for( QValueList<TypeDesc>::iterator it = argTypes.begin(); it != argTypes.end(); ++it ) {
      argRealTypes << asType->locateDecType( *it );
    }
  }
    
  QStringList::iterator def = argDefaults.begin();
  QStringList::iterator name = argNames.begin();
  QValueList<LocateResult>::iterator realType = argRealTypes.begin();
    
  while( realType != argRealTypes.end() ) {
    if( sig != "( " )
      sig += ", ";
      
    sig += (*realType)->fullNameChain();
    ++realType;
      
    if( name != argNames.end() ) {
      if( !(*name).isEmpty() ) sig += " " + *name;
      ++name;
    }
      
    if( def != argDefaults.end() && !(*def).isEmpty() ) {
      sig += " = " + *def;
      ++def;
    }
  }
    
  sig += " )";
  return sig;
}

bool SimpleTypeFunctionInterface::containsUndefinedTemplateParam( TypeDesc& desc, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  TypeDesc::TemplateParams& pm = desc.templateParams();
  SimpleTypeImpl::TemplateParamInfo::TemplateParam t;
  
  if( pm.isEmpty() && paramInfo.getParam( t, desc.name() ) )
    if( !t.value ) return true;
  
  if( desc.next() )
    if( containsUndefinedTemplateParam( *desc.next(), paramInfo ) )
      return true;
  
  for( TypeDesc::TemplateParams::iterator it = pm.begin(); it != pm.end(); ++it ) {
    if( containsUndefinedTemplateParam( **it, paramInfo ) ) return true;
  }
  
  return false;
}
  
void SimpleTypeFunctionInterface::resolveImplicitTypes( TypeDesc& argType, TypeDesc& gottenArgType, SimpleTypeImpl::TemplateParamInfo& paramInfo )
{
  if( argType.templateParams().isEmpty() ) {  ///Template-types may not be templates.
    SimpleTypeImpl::TemplateParamInfo::TemplateParam p;
    if( paramInfo.getParam( p, argType.name() ) && !p.value ) {
      ifVerbose( dbg() << "choosing \"" << gottenArgType.fullNameChain() << "\" as implicit template-parameter for \"" << argType.name() << "\"" << endl );
      p.value = gottenArgType;
      p.value.makePrivate();
      for( int d = 0; d < argType.pointerDepth(); d++ )
        p.value.decreasePointerDepth();
      
      paramInfo.addParam( p );
    }
  } else {
    if( argType.name() == gottenArgType.name() )
      resolveImplicitTypes( argType.templateParams(), gottenArgType.templateParams(), paramInfo );
  }
}

void SimpleTypeFunctionInterface::resolveImplicitTypes( TypeDesc::TemplateParams& argTypes, TypeDesc::TemplateParams& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  TypeDesc::TemplateParams::iterator it = argTypes.begin();
  TypeDesc::TemplateParams::iterator it2 = gottenArgTypes.begin();
  
  while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
    resolveImplicitTypes( **it, **it2, paramInfo );
    ++it;
    ++it2;
  }
}

void SimpleTypeFunctionInterface::resolveImplicitTypes( QValueList<TypeDesc>& argTypes, QValueList<TypeDesc>& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  QValueList<TypeDesc>::iterator it = argTypes.begin();
  QValueList<TypeDesc>::iterator it2 = gottenArgTypes.begin();
  
  while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
    resolveImplicitTypes( *it, *it2, paramInfo );
    ++it;
    ++it2;
  }
}

 void SimpleTypeFunctionInterface::appendNextFunction( SimpleType func ) {
   if( !func )return;
   if( (SimpleTypeImpl*)func.get() == (SimpleTypeImpl*)this ) return;
   if( m_nextFunction && m_nextFunction->asFunction() ) {
     m_nextFunction->asFunction()->appendNextFunction( func );
   } else {
     m_nextFunction = func;
   }
 }


//SimpleTypeCodeModel implementation

void SimpleTypeCodeModel::addAliasesTo( SimpleTypeNamespace* ns ) {
  const NamespaceModel* m = dynamic_cast<const NamespaceModel*>( m_item.data() );
  if( m ) {
    const NamespaceModel::NamespaceAliasModelList& namespaceAliases = m->namespaceAliases();
    const NamespaceModel::NamespaceImportModelList& namespaceImports = m->namespaceImports();
    for( NamespaceModel::NamespaceAliasModelList::const_iterator it = namespaceAliases.begin(); it != namespaceAliases.end(); ++it )
      ns->addAliasMap( it->name(), it->aliasName(), it->fileName(), true, true ); //should these really be symmetric?
    for( NamespaceModel::NamespaceImportModelList::const_iterator it = namespaceImports.begin(); it != namespaceImports.end(); ++it )
      ns->addAliasMap( "", it->name(), it->fileName(), true, false );
  }
}

SimpleTypeCodeModel::SimpleTypeCodeModel( ItemDom& item ) : m_item( item ) {
  CodeModelItem* i = &(*item);
  FunctionModel* m = dynamic_cast<FunctionModel*>( i );
  ClassModel* c = dynamic_cast<ClassModel*>( i );
  if( m ) {
    QStringList l = m->scope();
    l << m->name();
    setScope( l );
    return;
  }
  if( c ) {
    QStringList l = c->scope();
    l << c->name();
    setScope( l );
    return;
  }
  ifVerbose( dbg() << "code-model-item has an unsupported type: " << i->name() << endl );
}

ItemDom SimpleTypeCodeModel::locateModelContainer( class CodeModel* m, TypeDesc t, ClassDom cnt)
{
  if( !cnt ) {
    if( m->globalNamespace() ) {
      cnt = model_cast<ClassDom>( m->globalNamespace() );
    } else {
      return ItemDom();
    }
  }
  if( t ) {
    if( cnt->hasClass( t.name() ) ) {
      ClassList l = cnt->classByName( t.name() );
      if( !l.isEmpty() ) {
        if( t.next() )
          return locateModelContainer( m, *t.next(), l.front() );
        else
          return model_cast<ItemDom>( l.front() );
      }
    }
    NamespaceModel* ns = dynamic_cast<NamespaceModel*>(&(*cnt));
    if( ns ) {
      NamespaceDom n = ns->namespaceByName( t.name() );
      if( t.next() )
        return locateModelContainer( m, *t.next(), model_cast<ClassDom>( n ) );
      else
        return model_cast<ItemDom>( n );
    }
  }
  
  return ItemDom();
}

///Until header-parsing is implemented, this tries to find the class that is most related to this item
/*ClassDom SimpleTypeCodeModel::pickMostRelated( ClassList lst, QString fn ) {
  if( lst.isEmpty() ) return ClassDom();
  
  ClassDom best = lst.front();
  uint bestMatch = 0;
    //kdDebug() << "searching most related to " << fn << endl;
  
  for( ClassList::iterator it = lst.begin(); it != lst.end(); ++it ) {
    if( !(*it)->getSpecializationDeclaration().isEmpty() ) continue; ///Don't consider specialized classes
        //kdDebug() << "comparing " << (*it)->fileName() << endl;
    QString str = (*it)->fileName();
    uint len = str.length();
    if( fn.length() < len ) len = fn.length();
    
    uint matchLen = 0;
    for( uint a = 0; a < len; a++ ) {
      if( str[a] == fn[a] )
        matchLen++;
      else
        break;
    }
    
    if( matchLen > bestMatch ) {
            //kdDebug() << "picking " << str << endl;
      bestMatch = matchLen;
      best = *it;
    }
  }
  
    //kdDebug() << "picked " << best->fileName() << endl;
  if( !best->getSpecializationDeclaration().isEmpty() ) best = 0; ///only accept non-specialized classes
  return best;
}*/

/*QValueList<TypePointer> SimpleTypeCodeModel::findSpecializations( const QString& name ) {
	ClassModel* klass = dynamic_cast<ClassModel*> ( & (*m_item) );
	if( !klass ) {
		ifVerbose( dbg() << "\"" << str() << "\": search for member " << name.name() << " unsuccessful because the own type is invalid" << endl );
		return QValueList<TypePointer>();
	}
	
	ClassList l = klass->classByName( name.name() );
	
	if( !l.isEmpty() ) {
		ClassDom i = pickMostRelated( l, globalCurrentFile );
		if( i ) {
			ret.setBuildInfo( new CodeModelBuildInfo( model_cast<ItemDom>( i ), name, TypePointer( this ) ) );
			
			ret.memberType = MemberInfo::NestedType;
			ret.type = name;
		}
	}
	
	return QValueList<TypePointer>();
}*/


QValueList<TypePointer> SimpleTypeCodeModel::getMemberClasses( const TypeDesc& name ) {
	QValueList<TypePointer> ret;

	if( !m_item ) return ret;
	
	ClassModel* klass = dynamic_cast<ClassModel*> ( & (*m_item) );
	if( !klass ) {
	ifVerbose( dbg() << "\"" << str() << "\": search for member " << name.name() << " unsuccessful because the own type is invalid" << endl );
		return ret;
	}

	ClassList l = klass->classByName( name.name() );
	
	if( !l.isEmpty() ) {
		for( ClassList::iterator it = l.begin(); it != l.end(); ++it ) {
			CodeModelBuildInfo b( model_cast<ItemDom>( *it ), name, TypePointer( this ) );
			TypePointer r = b.build();
			if( r )
				ret << r;
		}
	}
    return ret;
}

template<class Item>
Item pickMostRelated( const HashedStringSet& includeFiles, const QValueList<Item>& list ) {
    if( list.isEmpty() ) return Item();
    
    for( typename QValueList<Item>::const_iterator it = list.begin(); it != list.end(); ++it ) {
        if( includeFiles[ (*it)->fileName() ] )
            return *it;
    }
    return list.front();
}

template<>
ClassDom pickMostRelated( const HashedStringSet& includeFiles, const QValueList<ClassDom>& list ) {
    if( list.isEmpty() ) return ClassDom();         ///@todo the current file must be preferred
    
    for( QValueList<ClassDom>::const_iterator it = list.begin(); it != list.end(); ++it ) {
        if( !(*it)->getSpecializationDeclaration().isEmpty() ) continue; ///Don't consider specialized classes
        if( includeFiles[ (*it)->fileName() ] )
            return *it;
    }

    
    if( !list.front()->getSpecializationDeclaration().isEmpty() ) return ClassDom(); ///Don't consider specialized classes
    return list.front();
}

SimpleTypeImpl::MemberInfo SimpleTypeCodeModel::findMember( TypeDesc name , MemberInfo::MemberType type )
{
  MemberInfo ret;
  ret.name = name.name();
  ret.memberType = MemberInfo::NotFound;
  if( !name || !m_item ) return ret;
  
  ClassModel* klass = dynamic_cast<ClassModel*> ( & (*m_item) );
  if( !klass ) {
    ifVerbose( dbg() << "\"" << str() << "\": search for member " << name.name() << " unsuccessful because the own type is invalid" << endl );
    return ret;
  }
  NamespaceModel* ns = dynamic_cast<NamespaceModel*>(klass);

  if( klass->hasVariable( name.name() )  && ( type & MemberInfo::Variable ) ) {
    ret.memberType = MemberInfo::Variable;
    VariableDom d = klass->variableByName( name.name() );
    if( d ) {
      ret.type = d->type();
      ret.type->setIncludeFiles( getIncludeFiles( d.data() ) );
      ret.decl.name = d->name();
      ret.decl.file = d->fileName();
      ret.decl.comment = d->comment();
      d->getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
      d->getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
    }
  } else if( klass->hasTypeAlias( name.name() ) && ( type & MemberInfo::Typedef ) ) {
    ret.memberType = MemberInfo::Typedef;
    TypeAliasList li = klass->typeAliasByName( name.name() );
    TypeAliasDom a = pickMostRelated( name.includeFiles(), li );
    
    if( a ) {
      ret.type = a->type();
      ret.type->setIncludeFiles( getIncludeFiles( a.data() ) );
	  ret.decl.name = a->name();
	  ret.decl.file = a->fileName();
	  ret.decl.comment = a->comment();
	  a->getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
	  a->getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
    }
  } else if( klass->hasEnum( name.name() ) && ( type & MemberInfo::Typedef ) ) {
    ret.memberType = MemberInfo::Typedef;
    EnumDom e = klass->enumByName( name.name() );
	  ret.type = TypeDesc( "const int" );
      ret.type->setIncludeFiles( getIncludeFiles( e.data() ) );
	  ret.decl.name = e->name();
	  ret.decl.file = e->fileName();
	  ret.decl.comment = e->comment();
	  e->getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
	  e->getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
  } else if ( klass->hasClass( name.name() ) && ( type & MemberInfo::NestedType ) ) {
    ClassList l = klass->classByName( name.name() );
    
    if( !l.isEmpty() ) {
        ClassDom i = pickMostRelated( name.includeFiles(), l );
      if( i ) {
        ret.setBuildInfo( new CodeModelBuildInfo( model_cast<ItemDom>( i ), name, TypePointer( this ) ) );
        
        ret.memberType = MemberInfo::NestedType;
        ret.type = name;
        ret.type->setIncludeFiles( getIncludeFiles( i.data() ) );
      }
    }
  } else if( klass->hasFunction( name.name() )  && ( type & MemberInfo::Function ) ) {
    ret.memberType = MemberInfo::Function;
    FunctionList l = klass->functionByName( name.name() );
    if( !l.isEmpty() && l.front() ) {
      ret.setBuildInfo( new SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo( l, name , TypePointer(this) ) );
      ret.type = l.front()->resultType();
        ret.type->setIncludeFiles( getIncludeFiles( l.front().data() ) );
      ret.type->increaseFunctionDepth();
    }
  } else if ( ns && ns->hasNamespace( name.name() )  && ( type & MemberInfo::Namespace ) ) {
      NamespaceDom n = ns->namespaceByName( name.name() );
    ret.setBuildInfo( new CodeModelBuildInfo( model_cast<ItemDom>( n), name, TypePointer( this ) ) );
    ret.memberType = MemberInfo::Namespace;
    ret.type = name;
      ret.type->setIncludeFiles( getIncludeFiles( n.data() ) );
  } else if( klass->hasFunctionDefinition( name.name() )  && ( type & MemberInfo::Function ) ) {
	  FunctionDefinitionList l = klass->functionDefinitionByName( name.name() );
	  for( FunctionDefinitionList::iterator it = l.begin(); it != l.end(); ++it ) {
		  if( !(*it)->scope().isEmpty() && (*it)->scope() != scope() ) continue; ///Only use definitions with empty scope or that are within this class
		  ret.setBuildInfo( new SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo( l, name, TypePointer(this) ) );
		  ret.type = l.front()->resultType();
    	  ret.type->setIncludeFiles( getIncludeFiles( l.front().data() ) );
		  ret.type->increaseFunctionDepth();
		  ret.memberType = MemberInfo::Function;
		  break;
	  }
  }

  if( ret.memberType == MemberInfo::NotFound ) {
    if( type & MemberInfo::Template ) {
      LocateResult s = findTemplateParam( name.name() );
      if( s ) {
        ret.memberType = MemberInfo::Template;
        ret.type = s;
		ret.decl.name = name.name();
	  if( m_item ) {
		  ret.decl.file = m_item->fileName();
		  m_item->getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
		  m_item->getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
	  }
      }
    }
  }

  chooseSpecialization( ret );
	
  return ret;
}

bool SimpleTypeCodeModel::findItem() {
  QString key = str();
  m_item = locateModelContainer( cppCompletionInstance->m_pSupport->codeModel(), str() );
  return (bool) m_item;
}

void SimpleTypeCodeModel::init() {
  if( scope().isEmpty() ) {
    m_item = cppCompletionInstance->m_pSupport->codeModel() ->globalNamespace();
  }else{
    findItem();
  }
}

DeclarationInfo SimpleTypeCodeModel::getDeclarationInfo() {
  DeclarationInfo ret;
  ItemDom i = item();
  ret.name = fullTypeResolved();
  if( i ) {
    ret.file = i->fileName();
    i->getStartPosition( &ret.startLine, &ret.startCol );
    i->getEndPosition( &ret.endLine, &ret.endCol );
    ret.comment = i->comment();
  }
  return ret;
}

QString SimpleTypeCodeModel::specialization() const {
	const ClassModel* klass = dynamic_cast<const ClassModel*>( m_item.data() );
	if( !klass ) return QString::null;
	return klass->getSpecializationDeclaration();
}

SimpleTypeImpl::TemplateParamInfo SimpleTypeCodeModel::getTemplateParamInfo() {
  TemplateParamInfo ret;
  
  if(m_item) {
    TemplateModelItem* ti = dynamic_cast<TemplateModelItem*> ( &( *m_item ) );
    TypeDesc::TemplateParams& templateParams = m_desc.templateParams();
    
    TemplateModelItem::ParamMap m =  ti->getTemplateParams();
    for( uint a = 0; a < m.size(); a++ ) {
      TemplateParamInfo::TemplateParam t;
      t.number = a;
      t.name = m[a].first;
      t.def = m[a].second;
      if( templateParams.count() > a )
        t.value = *templateParams[a];
      ret.addParam( t );
    }
  }
  
  return ret;
}

const LocateResult SimpleTypeCodeModel::findTemplateParam( const QString& name ) {
  if(m_item) {
    TemplateModelItem* ti = dynamic_cast<TemplateModelItem*> ( &( *m_item ) );
    TypeDesc::TemplateParams& templateParams = m_desc.templateParams();
    int pi = ti->findTemplateParam( name );
    if( pi != -1 && (int)templateParams.count() > pi ) {
      return *templateParams[pi];
    } else {
      if( pi != -1 && !ti->getParam( pi ).second.isEmpty() ) {
        QString def = ti->getParam( pi ).second;
        ifVerbose( dbg() << "\"" << str() << "\": using default-template-parameter \"" << def << "\" for " << name << endl );
	  	return TypeDesc( def );
      } else if( pi != -1 ) {
        ifVerbose( dbg() << "\"" << str() << "\": template-type \"" << name << "\" has no pameter! " << endl );
      }
    }
  }
  return LocateResult();
};

QValueList<LocateResult> SimpleTypeCodeModel::getBases() {
  Debug d( "#getbases#" );
  if( !d || !safetyCounter ) {
	  //ifVerbose( dbg() << "\"" << str() << "\": recursion to deep while getting bases" << endl );
    return QValueList<LocateResult>();
  }
  
  QValueList<LocateResult> ret;
  
  ClassModel* klass;
  
  if( !m_item || ( klass = dynamic_cast<ClassModel*>( &(*m_item) ) ) == 0 ) return ret;
  
  QStringList parents = klass->baseClassList();
  for ( QStringList::Iterator it = parents.begin(); it != parents.end(); ++it )
  {
    ret << locateDecType( *it , LocateBase );
  }
  
  return ret;
}

TypePointer SimpleTypeCodeModel::CodeModelBuildInfo::build() {
  TypePointer tp = new SimpleTypeCachedCodeModel( m_item );
  tp->parseParams( m_desc );
  if( m_parent ) tp->setParent( m_parent->bigContainer() );
  return tp;
}

//SimpleTypeCodeModelFunction implementation
TypeDesc SimpleTypeCodeModelFunction::getReturnType() {
  if( item() ) {
    if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
      return m->resultType();
    }
  }
  
  return TypeDesc();
}

bool SimpleTypeCodeModelFunction::isConst() {
  if( asFunctionModel() )
    return asFunctionModel()->isConstant();
  
  return false;
}


QValueList<TypeDesc> SimpleTypeCodeModelFunction::getArgumentTypes() {
  QValueList<TypeDesc> ret;
  
  if( item() ) {
    if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
      ArgumentList l = m->argumentList();
      for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
        ret << TypeDesc( (*it)->type() );
    }
  }
  
  return ret;
}

QStringList SimpleTypeCodeModelFunction::getArgumentNames() {
  QStringList ret;
  
  if( item() ) {
    if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
      ArgumentList l = m->argumentList();
      for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
        ret << (*it)->name();
    }
  }
  
  return ret;
}

QStringList SimpleTypeCodeModelFunction::getArgumentDefaults() {
  QStringList ret;
  
  if( item() ) {
    if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
      ArgumentList l = m->argumentList();
      for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
        ret << (*it)->defaultValue();
    }
  }
  
  return ret;
}


//SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo implementation

SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo::CodeModelFunctionBuildInfo( FunctionDefinitionList items, TypeDesc& desc, TypePointer parent ) : m_desc( desc ), m_parent( parent ) {
  
  for( FunctionDefinitionList::iterator it = items.begin(); it != items.end(); ++it ) {
    m_items << model_cast<FunctionDom>( *it );
  }
}

TypePointer SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo::build() {
  QValueList<TypePointer> ret;
  TypePointer last;
  for( FunctionList::iterator it = m_items.begin(); it != m_items.end(); ++it ) {
    TypePointer tp = new SimpleTypeCodeModelFunction( model_cast<ItemDom>( *it ) );
    tp->takeTemplateParams( m_desc );
    tp->descForEdit().increaseFunctionDepth();
    tp->setParent( m_parent->bigContainer() );
    if( last && last->asFunction() ) last->asFunction()->appendNextFunction( SimpleType( tp) );
    last = tp;
    ret << tp;
  }
  
  if( ret.isEmpty() ) {
    ifVerbose( dbg() << "error" << endl );
    return TypePointer();
  } else
    return ret.front();
}

//SimpleTypeCatalogFunction implementation
TypeDesc SimpleTypeCatalogFunction::getReturnType() {
  if( tag() ) {
    return tagType( tag() );
  }
  
  return TypeDesc();
}

bool SimpleTypeCatalogFunction::isConst () {
  Tag t = tag();
  CppFunction<Tag> tagInfo( t );
  return tagInfo.isConst();
}

QStringList SimpleTypeCatalogFunction::getArgumentNames() {
  QStringList ret;
  Tag t = tag();
  CppFunction<Tag> tagInfo( t );
  return tagInfo.argumentNames();
}

QValueList<TypeDesc> SimpleTypeCatalogFunction::getArgumentTypes() {
  QValueList<TypeDesc> ret;
  Tag t = tag();
  CppFunction<Tag> tagInfo( t );
  QStringList arguments = tagInfo.arguments();
  for( QStringList::iterator it = arguments.begin(); it != arguments.end(); ++it )
    ret << TypeDesc( *it );
  return ret;
}

//SimpleTypeCatalogFunction::CatalogFunctionBuildInfo implementation

TypePointer SimpleTypeCatalogFunction::CatalogFunctionBuildInfo::build() {
  QValueList<TypePointer> ret;
  TypePointer last;
  for( QValueList<Tag>::iterator it = m_tags.begin(); it != m_tags.end(); ++it ) {
    TypePointer tp = new SimpleTypeCatalogFunction( *it );
    tp->takeTemplateParams( m_desc );
    tp->descForEdit().increaseFunctionDepth();
    if( m_parent ) tp->setParent( m_parent->bigContainer() );
    if( last && last->asFunction() ) last->asFunction()->appendNextFunction( SimpleType( tp) );
    last = tp;
    ret << tp;
  }
  
  if( ret.isEmpty() ) {
    ifVerbose( dbg() << "error" << endl );
    return TypePointer();
  }
  return ret.front();
}
 
// kate: indent-mode csands; tab-width 4;
