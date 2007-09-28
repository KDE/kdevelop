/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "cppcodecompletionmodel.h"

#include <QIcon>
#include <QMetaType>
#include <QTextFormat>
#include <QBrush>
#include <kdebug.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kiconloader.h>
#include <khtmlview.h>


#include "duchainbuilder/cppduchain.h"
#include "duchainbuilder/typeutils.h"

#include "expressionparser/overloadresolutionhelper.h"

#include <declaration.h>
#include "cpptypes.h"
#include "typeutils.h"
#include <classfunctiondeclaration.h>
#include <ducontext.h>
#include <duchain.h>
#include <namespacealiasdeclaration.h>
#include <parsingenvironment.h>
#include <editorintegrator.h>
#include <duchainlock.h>
#include <duchainbase.h>
#include <topducontext.h>
#include "dumpchain.h"
#include "codecompletioncontext.h"
#include "navigationwidget.h"
#include "preprocessjob.h"


using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

//Returns the type as which a declaration in the completion-list should be interpreted, which especially means that it returns the return-type of a function.
AbstractType::Ptr effectiveType( Declaration* decl )
{
  if( !decl || !decl->abstractType() )
    return AbstractType::Ptr();
  
  if( decl->type<FunctionType>() )
    return decl->type<FunctionType>()->returnType();

  return decl->abstractType();
}

CppCodeCompletionModel::CppCodeCompletionModel( QObject * parent )
  : CodeCompletionModel(parent)
{
  QString allIconNames = "CVprotected_var CVprivate_var protected_union protected_enum private_struct CVprotected_slot private_enum CVprotected_signal CVprivate_slot protected_class private_class private_union protected_function private_function signal CVpublic_var enum class CVpublic_slot union typedef function struct protected_field private_field field";

  foreach( QString iconName, allIconNames.split(" ") )
    m_icons[iconName] = KIconLoader::global()->loadIcon(iconName, K3Icon::Small);
  
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
}

void CppCodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
  Q_UNUSED(invocationType)
  
  m_navigationWidgets.clear();

  KUrl url = view->document()->url();
  TopDUContext* top = 0;
  
  if( !KDevelop::DUChain::lock()->lockForRead(400) ) {
    kDebug(9007) << "could not lock du-chain in time" << endl;
    return;
  }
  
  ParsingEnvironment* env = PreprocessJob::createStandardEnvironment();
  top = KDevelop::DUChain::self()->chainForDocument(url, env);
  delete env;

  if( !top ) {
    kDebug(9007) << "Could not find perfectly matching version of " << url << " for completion";
    top = DUChain::self()->chainForDocument(url);
  }

  if(top && top->flags() & TopDUContext::ProxyContextFlag)
  {
    if(!top->importedParentContexts().isEmpty())
    {
      if(top->importedParentContexts().count() != 1)
        kDebug(9007) << "WARNING: Proxy-context has more than one content-contexts, this should never happen";
      
      top = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().data());
      
      if(!top)
        kDebug(9007) << "WARNING: Proxy-context had invalid content-context";
      
    } else {
      kDebug(9007) << "ERROR: Proxy-context has no content-context";
    }
  }
  
  if (top) {
    kDebug(9007) << "completion invoked for context" << (DUContext*)top;

    if( top->parsingEnvironmentFile()->modificationRevision() != EditorIntegrator::modificationRevision(url) ) {
      kDebug(9007) << "Found context is not current. Its revision is " << top->parsingEnvironmentFile()->modificationRevision() << " while the document-revision is " << EditorIntegrator::modificationRevision(url);
    }
    
    DUContextPointer thisContext;
    {
      thisContext = top->findContextAt(range.start());

       kDebug(9007) << "context is set to" << thisContext.data();
        if( thisContext ) {
/*          kDebug( 9007 ) << "================== duchain for the context =======================";
          DumpChain dump;
          dump.dump(thisContext.data());*/
        } else {
          kDebug( 9007 ) << "================== NO CONTEXT FOUND =======================";
          m_declarations.clear();
          m_navigationWidgets.clear();
          reset();
          return;
        }
    }

    DUChain::lock()->releaseReadLock();
    
    setContext(thisContext, range.start(), view);
  } else {
    kDebug(9007) << "Completion invoked for unknown context. Document:" << url << ", Known documents:" << DUChain::self()->documents();
    DUChain::lock()->releaseReadLock();
  }
}

void CppCodeCompletionModel::createArgumentList(const CompletionItem& item, QString& ret, QList<QVariant>* highlighting ) const
{
  ///@todo also highlight the matches of the previous arguments, they are given by ViableFunction
  Declaration* dec(item.declaration.data());

  Cpp::CodeCompletionContext::Function f;

  if( item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && item.completionContext->functions().count() > item.listOffset )
    f = item.completionContext->functions()[item.listOffset];
  
  int textFormatStart = 0;
  QTextFormat normalFormat;
  QTextFormat highlightFormat(normalFormat);
  highlightFormat.setBackground(Qt::yellow);
  
  AbstractFunctionDeclaration* decl = dynamic_cast<AbstractFunctionDeclaration*>(dec);
  CppFunctionType::Ptr functionType = dec->type<CppFunctionType>();
  if (functionType && decl) {

    QStringList paramNames = decl->parameterNames();
    QStringList defaultParams = decl->defaultParameters();

    QStringList::const_iterator paramNameIt = paramNames.begin();
    QStringList::const_iterator defaultParamIt = defaultParams.begin();

    int firstDefaultParam = functionType->arguments().count() - defaultParams.count();
    
    ret = "(";
    bool first = true;
    int num = 0;
    foreach (const AbstractType::Ptr& argument, functionType->arguments()) {
      if (first)
        first = false;
      else
        ret += ", ";

      if( f.function.isValid() && num == f.matchedArguments )
      {
        if( highlighting && ret.length() != textFormatStart )
        {
          *highlighting <<  QVariant(textFormatStart);
          *highlighting << QVariant(ret.length());
          *highlighting << normalFormat;
          textFormatStart = ret.length();
        }
        ///@todo use real highlighting instead of this ugly sign
        ret += "    > "; //Currently highlighting does not work, so we highlight the item using this ugly sign
      }
      
      if (argument)
        ret += argument->toString();
      else
        ret += "<incomplete type>";

      if( paramNameIt != paramNames.end() && !(*paramNameIt).isEmpty() )
        ret += " " + *paramNameIt;
      
      if( f.function.isValid() && num == f.matchedArguments  )
      {
        ret += " <    "; //Currently highlighting does not work, so we highlight the item using this ugly sign
        if( highlighting && ret.length() != textFormatStart )
        {
          *highlighting <<  QVariant(textFormatStart);
          *highlighting << QVariant(ret.length());
          *highlighting << highlightFormat;
          textFormatStart = ret.length();
        }
      }
      
      if( num >= firstDefaultParam ) {
        ret += " = " + *defaultParamIt;
        ++defaultParamIt;
      }
      
      ++num;
    }
    ret += ')';
    if( functionType->isConstant() )
      ret += " const";

    if( highlighting && ret.length() != textFormatStart ) {
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length());
      *highlighting << normalFormat;
      textFormatStart = ret.length();
    }
    
    return;
  }
}

QVariant CppCodeCompletionModel::getIncludeData(const QModelIndex& index, int role) const {
  quint32 dataIndex = index.internalId();

  if( dataIndex >= (quint32)m_declarations.size() )
    return QVariant();

  const CompletionItem& completionItem(m_declarations[dataIndex]);
  const Cpp::IncludeItem& item( completionItem.includeItem );

  switch (role) {
    case IsExpandable:
      return QVariant(true);
    case ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(item);
      m_navigationWidgets[&completionItem] = nav;

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav->view());
       return v;
    }
    case AccessibilityNext:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&completionItem];
      if( w )
        w->next();
    }
    break;
    case AccessibilityPrevious:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&completionItem];
      if( w )
        w->previous();
    }
    break;
    case AccessibilityAccept:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&completionItem];
      if( w )
        w->accept();
    }
    break;
    case InheritanceDepth:
      return item.pathNumber;
    case Qt::DisplayRole:
      switch (index.column()) {
        case Prefix:
          if(item.isDirectory)
            return QVariant("directory");
          else
            return QVariant("file");
        case Name: {
/*          QString indentation;
          for( int a = 0; a < item.pathNumber; a++ )
            indentation += ' ';*/
        
          return /*indentation + */item.name;
        }
      }
      break;
    case ItemSelected:
    {
//      KUrl path = item.basePath;
//      path.addPath("/" + item.name);
//      return QVariant(path.prettyUrl());
      return QVariant( Cpp::NavigationWidget::shortDescription(item) );
    }
  }
  
  return QVariant();
}

QVariant CppCodeCompletionModel::data(const QModelIndex& index, int role) const
{
  quint32 dataIndex = index.internalId();

  if( dataIndex >= (quint32)m_declarations.size() )
    return QVariant();

  DUChainReadLocker lock(DUChain::lock());

  Declaration* dec = const_cast<Declaration*>( m_declarations[dataIndex].declaration.data() );
  if (!dec) {
    if(m_completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::IncludeListAccess)
      return getIncludeData(index, role);
  
    kDebug(9007) <<  "code-completion model item" << dataIndex << ": Du-chain item is deleted";
    return QVariant();
  }

  const CompletionItem& item(m_declarations[dataIndex]);

  bool isArgumentHint = false;
  if( item.completionContext && item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess )
    isArgumentHint = true;

  switch (role) {
    case AccessibilityNext:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&item];
      if( w )
        w->next();
    }
    break;
    case AccessibilityPrevious:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&item];
      if( w )
        w->previous();
    }
    break;
    case AccessibilityAccept:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&item];
      if( w )
        w->accept();
    }
    break;
    case BestMatchesCount:
      return QVariant(5);
    break;
    case InheritanceDepth:
      return item.inheritanceDepth;
    break;
    case SetMatchContext:
      m_currentMatchContext = item;
      return QVariant(1);
    case MatchQuality:
    {
      if( m_currentMatchContext.declaration && m_currentMatchContext.completionContext && m_currentMatchContext.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && m_currentMatchContext.listOffset < m_currentMatchContext.completionContext->functions().count() )
      {
        Cpp::CodeCompletionContext::Function f( m_currentMatchContext.completionContext->functions()[m_currentMatchContext.listOffset] );

        if( f.function.isValid() && f.function.isViable() && f.function.declaration() && f.function.declaration()->type<CppFunctionType>() && f.function.declaration()->type<CppFunctionType>()->arguments().count() > f.matchedArguments ) {
          Cpp::TypeConversion conv;

          ///@todo fill the lvalue-ness correctly
          int quality = ( conv.implicitConversion( effectiveType(dec), f.function.declaration()->type<CppFunctionType>()->arguments()[f.matchedArguments], true )  * 10 ) / Cpp::MaximumConversionResult;
          return QVariant(quality);
        }else{
          //kDebug(9007) << "MatchQuality requested with invalid match-context";
        }
      } else {
        //kDebug(9007) << "MatchQuality requested with invalid match-context";
      }
    }
    return QVariant();
    case ArgumentHintDepth:
      if( isArgumentHint )
        return item.completionContext->depth();
      else
        return QVariant();
    case ItemSelected:
       return QVariant(Cpp::NavigationWidget::shortDescription(dec));
    case IsExpandable:
      return QVariant(true);
    case ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(dec);
      m_navigationWidgets[&item] = nav;

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav->view());
       return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case Prefix:
        {
          QString indentation;
          int depth = item.inheritanceDepth;
          if( depth > 1000 )
            depth-=1000;
          for( int a = 0; a < depth; a++ )
            indentation += " ";

          if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(dec) ) {
            if( alias->identifier().isEmpty() ) {
              return indentation + "using namespace";/* " + alias->importIdentifier().toString();*/
            } else {
              return indentation + "namespace";/* " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();*/
            }
          }
          
          if( dec->isTypeAlias() )
            indentation += "typedef ";
          
          if( dec->kind() == Declaration::Type && !dec->type<CppFunctionType>() && !dec->isTypeAlias() ) {
            if (CppClassType::Ptr classType =  dec->type<CppClassType>())
              switch (classType->classType()) {
                case CppClassType::Class:
                  return indentation + "class";
                  break;
                case CppClassType::Struct:
                  return indentation + "struct";
                  break;
                case CppClassType::Union:
                  return indentation + "union";
                  break;
              }
            return QVariant();
          }
          if (dec->abstractType()) {
            if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
              ClassFunctionDeclaration* funDecl = dynamic_cast<ClassFunctionDeclaration*>(dec);
              
              if (functionType->returnType())
                return indentation + functionType->returnType()->toString();
              else if(funDecl && funDecl->isConstructor() )
                return indentation + "<constructor>";
              else if(funDecl && funDecl->isDestructor() )
                return indentation + "<destructor>";
              else
                return indentation + "<incomplete type>";

            } else {
              return indentation + dec->abstractType()->toString();
            }
          } else {
            return indentation + "<incomplete type>";
          }
        }

        case Scope: {
          //The scopes are not needed
          return QVariant();
/*          QualifiedIdentifier id = dec->qualifiedIdentifier();
          if (id.isEmpty())
            return QVariant();
          id.pop();
          if (id.isEmpty())
            return QVariant();
          return id.toString() + "::";*/
        }

        case Name:
          if (dec->identifier().toString().isEmpty())
            return "<unknown>";
          else
            return dec->identifier().toString();

        case Arguments:
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            QString ret;
            createArgumentList(item, ret, 0);
            return ret;
          }
        break;
        case Postfix:
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            return functionType->cvString();
          }
          break;
      }
      break;
    case HighlightingMethod:
    if( index.column() == Arguments ) {
      if( item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess )
        return QVariant(CustomHighlighting);
      else
        return QVariant();
      break;
    }
    break;

    case CustomHighlight:
    if( index.column() == Arguments && index.column() == Arguments && item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
      QString ret;
      QList<QVariant> highlight;
      createArgumentList(item, ret, &highlight);
      return QVariant(highlight);
    }
    break;
    case Qt::DecorationRole:
    case CompletionRole: {
      CompletionProperties p;
      if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(dec)) {
        switch (member->accessPolicy()) {
          case Declaration::Public:
            p |= Public;
            break;
          case Declaration::Protected:
            p |= Protected;
            break;
          case Declaration::Private:
            p |= Private;
            break;
        }

        if (member->isStatic())
          p |= Static;
        if (member->isAuto())
          ;//TODO
        if (member->isFriend())
          p |= Friend;
        if (member->isRegister())
          ;//TODO
        if (member->isExtern())
          ;//TODO
        if (member->isMutable())
          ;//TODO
      }

      if (AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(dec)) {
        if (function->isVirtual())
          p |= Virtual;
        if (function->isInline())
          p |= Inline;
        if (function->isExplicit())
          ;//TODO
      }

      if( dec->isTypeAlias() )
        p |= TypeAlias;

      if (dec->abstractType()) {
        if (CppCVType* cv = dynamic_cast<CppCVType*>(dec->abstractType().data())) {
          if (cv->isConstant())
            p |= Const;
          if (cv->isVolatile())
            ;//TODO
          }

        switch (dec->abstractType()->whichType()) {
          case AbstractType::TypeIntegral:
            if (dec->type<CppEnumerationType>())
              p |= Enum;
            else
              p |= Variable;
            break;
          case AbstractType::TypePointer:
            p |= Variable;
            break;
          case AbstractType::TypeReference:
            p |= Variable;
            break;
          case AbstractType::TypeFunction:
            p |= Function;
            break;
          case AbstractType::TypeStructure:
            if (CppClassType::Ptr classType =  dec->type<CppClassType>())
              switch (classType->classType()) {
                case CppClassType::Class:
                  p |= Class;
                  break;
                case CppClassType::Struct:
                  p |= Struct;
                  break;
                case CppClassType::Union:
                  p |= Union;
                  break;
              }
            break;
          case AbstractType::TypeArray:
            p |= Variable;
            break;
          case AbstractType::TypeAbstract:
            // TODO
            break;
        }

        if( dec->kind() == Declaration::Instance )
          p |= Variable;
      }

      /*
      LocalScope      = 0x80000,
      NamespaceScope  = 0x100000,
      GlobalScope     = 0x200000,
      */
      if( dec->context()->type() == DUContext::Global )
        p |= GlobalScope;
      else if( dec->context()->type() == DUContext::Namespace )
        p |= NamespaceScope;
      else if( dec->context()->type() != DUContext::Class )
        p |= LocalScope;

      if( role == CompletionRole ) {
        return (int)p;
      } else {
        ///Assign mini-icons
        QString iconName;

        if( (p & Variable) )
          iconName = "CVprotected_var";
        else if( (p & Variable) && (p & Protected) )
          iconName = "CVprotected_var";
        else if( (p & Variable) && (p & Private) )
          iconName = "CVprivate_var";
        else if( (p & Union) && (p & Protected) )
          iconName = "protected_union";
        else if( (p & Enum) && (p & Protected) )
          iconName = "protected_enum";
        else if( (p & Struct) && (p & Private) )
          iconName = "private_struct";
        else if( (p & Slot) && (p & Protected) )
          iconName = "CVprotected_slot";
        else if( (p & Enum) && (p & Private) )
          iconName = "private_enum";
        else if( (p & Signal) && (p & Protected) )
          iconName = "CVprotected_signal";
        else if( (p & Slot) && (p & Private) )
          iconName = "CVprivate_slot";
        else if( (p & Class) && (p & Protected) )
          iconName = "protected_class";
        else if( (p & Class) && (p & Private) )
          iconName = "private_class";
        else if( (p & Union) && (p & Private) )
          iconName = "private_union";
        else if( (p & TypeAlias) && ((p & Const) /*||  (p & Volatile)*/) )
          iconName = "CVtypedef";
        else if( (p & Function) && (p & Protected) )
          iconName = "protected_function";
        else if( (p & Function) && (p & Private) )
          iconName = "private_function";
        else if( p & Signal )
          iconName = "signal";
        else if( p & Variable )
          iconName = "CVpublic_var";
        else if( p & Enum )
          iconName = "enum";
        else if( p & Class )
          iconName = "class";
        else if( p & Slot )
          iconName = "CVpublic_slot";
        else if( p & Union )
          iconName = "union";
        else if( p & TypeAlias )
          iconName = "typedef";
        else if( p & Function )
          iconName = "function";
        else if( p & Struct )
          iconName = "struct";
        else if( p & Protected )
          iconName = "protected_field";
        else if( p & Private )
          iconName = "private_field";
        else
          iconName = "field";

        if( index.column() == Icon ) {
          lock.unlock();
          return QVariant( m_icons[iconName] );
        }
        break;

      }
    }

    case ScopeIndex:
      return static_cast<int>(reinterpret_cast<long>(dec->context()));
  }

  return QVariant();
}

QModelIndex CppCodeCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
  if (row < 0 || row >= m_declarations.count() || column < 0 || column >= ColumnCount || parent.isValid())
    return QModelIndex();

  QModelIndex ret = createIndex(row, column, row);

  return ret;
}

int CppCodeCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if (parent.isValid())
    return 0;

  return m_declarations.count();
}

void CppCodeCompletionModel::setContext(DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view)
{
  m_context = context;
  Q_ASSERT(m_context);
  //@todo move completion-context-building into another thread

  m_declarations.clear();

  //Compute the text we should complete on
  KTextEditor::Document* doc = view->document();
  if( !doc ) {
    kDebug(9007) << "No document for completion";
    return;
  }

  KTextEditor::Range range;
  QString text;
  {
    DUChainReadLocker lock(DUChain::lock());
    range = KTextEditor::Range(context->textRange().start(), position);

    text = doc->text(range);
  }

  if( text.isEmpty() ) {
    kDebug(9007) << "no text for context";
    return;
  }

  if( position.column() == 0 ) //Seems like when the cursor is a the beginning of a line, kate does not give the \n
    text += '\n';

  Cpp::CodeCompletionContext::Ptr completionContext( new Cpp::CodeCompletionContext( context, text ) );
  m_completionContext = completionContext;

  typedef QPair<Declaration*, int> DeclarationDepthPair;
  
  if( completionContext->isValid() ) {
    DUChainReadLocker lock(DUChain::lock());

    if( completionContext->memberAccessContainer().isValid() ||completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::StaticMemberChoose )
    {
      QList<DUContext*> containers = completionContext->memberAccessContainers();
      if( !containers.isEmpty() ) {
        foreach(DUContext* ctx, containers)
          foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( ctx->allDeclarations(ctx->textRange().end(), false) ) )
            m_declarations << CompletionItem( decl.first, completionContext, decl.second );
      } else {
        kDebug(9007) << "CppCodeCompletionModel::setContext: bad container-type";
      }
    } else if( completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::IncludeListAccess ) {
      //Include-file completion
      m_declarations.clear();
      int cnt = 0;
      QList<Cpp::IncludeItem> allIncludeItems = completionContext->includeItems();
      foreach(const Cpp::IncludeItem& includeItem, allIncludeItems) {
        CompletionItem completionItem;
        completionItem.includeItem = includeItem;
        m_declarations << completionItem;
        ++cnt;
      }
      kDebug(9007) << "Added " << cnt << " include-files to completion-list";
    } else {
      //Show all visible declarations
      m_declarations.clear();
      foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( m_context->allDeclarations(m_context->type() == DUContext::Class ? m_context->textRange().end() : position) ) )
        m_declarations << CompletionItem( decl.first, completionContext, decl.second );
      kDebug(9007) << "CppCodeCompletionModel::setContext: using all declarations visible:" << m_declarations.count();
    }

    ///Find all recursive function-calls that should be shown as call-tips
    Cpp::CodeCompletionContext::Ptr parentContext = completionContext;
    do {
      parentContext = parentContext->parentContext();
      if( parentContext ) {
        if( parentContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
          int num = 0;
          foreach( Cpp::CodeCompletionContext::Function function, parentContext->functions() ) {
            m_declarations << CompletionItem( function.function.declaration(), parentContext, 0, num );
            ++num;
          }
        } else {
          kDebug(9007) << "parent-context has non function-call access type";
        }
      }
    } while( parentContext );
  } else {
    kDebug(9007) << "CppCodeCompletionModel::setContext: Invalid code-completion context";
  }

  // TODO maybe one day just behave like a nice model and call insert rows etc.
  reset();
}

#include "cppcodecompletionmodel.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
