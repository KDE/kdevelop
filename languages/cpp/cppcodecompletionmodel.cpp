/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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


#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"

#include "cppduchain/overloadresolutionhelper.h"

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
#include <duchainutils.h>
#include "cppcodecompletionworker.h"
#include "cpplanguagesupport.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

DUContext* getFunctionContext(Declaration* decl) {
  DUContext* internal = decl->internalContext();
  if( !internal )
    return 0;
  if( internal->type() == DUContext::Function )
    return internal;
  foreach( DUContextPointer ctx, internal->importedParentContexts() ) {
    if( ctx )
      if( ctx->type() == DUContext::Function )
        return ctx.data();
  }
  return 0;
}

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
  : CodeCompletionModel2(parent)
  , m_mutex(new QMutex)
  , m_worker(new CodeCompletionWorker(this))
{
  qRegisterMetaType<QList<CompletionTreeElement> >("QList<KSharedPtr<CompletionTreeElement> >");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");

  connect(this, SIGNAL(completionsNeeded(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), m_worker, SLOT(computeCompletions(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), Qt::QueuedConnection);

  //We connect directly, so we can do the pre-grouping within the background thread
  connect(m_worker, SIGNAL(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), this, SLOT(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), Qt::QueuedConnection);

  m_worker->start();
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
  // Let it leak...??
  m_worker->setParent(0L);
  m_worker->quit();

  delete m_mutex;
}

void CppCodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
  Q_UNUSED(invocationType)

  m_navigationWidgets.clear();
  m_completionItems.clear();

  reset();

  m_worker->abortCurrentCompletion();

  KUrl url = view->document()->url();

  if( !KDevelop::DUChain::lock()->lockForRead(400) ) {
    kDebug(9007) << "could not lock du-chain in time";
    return;
  }

  TopDUContext* top = CppLanguageSupport::self()->standardContext( url );
  m_currentTopContext = TopDUContextPointer(top);

  if (top) {
    kDebug(9007) << "completion invoked for context" << (DUContext*)top;

    if( top->parsingEnvironmentFile()->modificationRevision() != EditorIntegrator::modificationRevision(url.prettyUrl()) ) {
      kDebug(9007) << "Found context is not current. Its revision is " << top->parsingEnvironmentFile()->modificationRevision() << " while the document-revision is " << EditorIntegrator::modificationRevision(url.prettyUrl());
    }

    DUContextPointer thisContext;
    {
      thisContext = top->findContextAt(SimpleCursor(range.start()));

       kDebug(9007) << "context is set to" << thisContext.data();
        if( thisContext ) {
/*          kDebug( 9007 ) << "================== duchain for the context =======================";
          DumpChain dump;
          dump.dump(thisContext.data());*/
        } else {
          kDebug( 9007 ) << "================== NO CONTEXT FOUND =======================";
          m_completionItems.clear();
          m_navigationWidgets.clear();
          reset();
          return;
        }
    }

    DUChain::lock()->releaseReadLock();

    emit completionsNeeded(thisContext, range.start(), view);

  } else {
    kDebug(9007) << "Completion invoked for unknown context. Document:" << url << ", Known documents:" << DUChain::self()->documents();
    DUChain::lock()->releaseReadLock();
  }
}

void CppCodeCompletionModel::foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > items, void* completionContext)
{
  m_completionItems = items;
  m_completionContext = KSharedPtr<Cpp::CodeCompletionContext>((Cpp::CodeCompletionContext*)completionContext);
  reset();
  
/*  if (completionContext == m_completionContext.data()) {
    if( !m_completionItems.isEmpty() ) {
      beginInsertRows(QModelIndex(), m_completionItems.count(), m_completionItems.count() + items.count() - 1);
      m_completionItems += items;
      endInsertRows();
    } else {*/
/*    }
  }*/
  
}

void CppCodeCompletionModel::setCompletionContext(KSharedPtr<Cpp::CodeCompletionContext> completionContext)
{
  QMutexLocker lock(m_mutex);
  m_completionContext = completionContext;
}

KSharedPtr<Cpp::CodeCompletionContext> CppCodeCompletionModel::completionContext() const
{
  QMutexLocker lock(m_mutex);
  return m_completionContext;
}

void CppCodeCompletionModel::createArgumentList(const CompletionItem& item, QString& ret, QList<QVariant>* highlighting ) const
{
  ///@todo also highlight the matches of the previous arguments, they are given by ViableFunction
  Declaration* dec(item.declaration.data());

  Cpp::CodeCompletionContext::Function f;

  if( item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && item.completionContext->functions().count() > item.listOffset )
    f = item.completionContext->functions()[item.listOffset];

  int textFormatStart = 0;
  QTextFormat normalFormat(QTextFormat::CharFormat);
  QTextFormat highlightFormat; //highlightFormat is invalid, so kate uses the match-quality dependent color.

  AbstractFunctionDeclaration* decl = dynamic_cast<AbstractFunctionDeclaration*>(dec);
  CppFunctionType::Ptr functionType = dec->type<CppFunctionType>();
  if (functionType && decl) {

    QVector<Declaration*> parameters;
    if( getFunctionContext(dec) )
      parameters = getFunctionContext(dec)->localDeclarations();
    
    QStringList defaultParams = decl->defaultParameters();

    QVector<Declaration*>::const_iterator paramNameIt = parameters.begin();
    QStringList::const_iterator defaultParamIt = defaultParams.begin();

    int firstDefaultParam = functionType->arguments().count() - defaultParams.count();

    ret = "(";
    bool first = true;
    int num = 0;

    const QList<Cpp::ViableFunction::ParameterConversion>& conversions = f.function.parameterConversions();
    QList<Cpp::ViableFunction::ParameterConversion>::const_iterator parameterConversion = conversions.begin();

    foreach (const AbstractType::Ptr& argument, functionType->arguments()) {
      if (first)
        first = false;
      else
        ret += ", ";

      bool doHighlight = false;
      QTextFormat doFormat = normalFormat;

      if( ( f.function.isValid() && num == f.matchedArguments ) )
      {
        doHighlight = true;
        doFormat = highlightFormat;

      } else if( num < f.matchedArguments )
      {
        doHighlight = true;
        doFormat = QTextFormat( QTextFormat::CharFormat );

        if( parameterConversion != conversions.end() ) {
          //Interpolate the color
          quint64 badMatchColor = 0xff7777ff; //Full blue
          quint64 goodMatchColor = 0xff77ff77; //Full green

          uint totalColor = (badMatchColor*(Cpp::MaximumConversionResult-(*parameterConversion).rank) + goodMatchColor*(*parameterConversion).rank)/Cpp::MaximumConversionResult;

          doFormat.setBackground( QBrush(totalColor) );

          ++parameterConversion;
        }
      }

      if( doHighlight )
      {
        if( highlighting && ret.length() != textFormatStart )
        {
          //Add a default-highlighting for the passed text
          *highlighting <<  QVariant(textFormatStart);
          *highlighting << QVariant(ret.length() - textFormatStart);
          *highlighting << QVariant(normalFormat);
          textFormatStart = ret.length();
        }
      }

      if (argument)
        ret += argument->toString();
      else
        ret += "<incomplete type>";

      if( paramNameIt != parameters.end() && !(*paramNameIt)->identifier().isEmpty() )
        ret += " " + (*paramNameIt)->identifier().toString();

      if( doHighlight  )
      {
        if( highlighting && ret.length() != textFormatStart )
        {
          *highlighting <<  QVariant(textFormatStart);
          *highlighting << QVariant(ret.length() - textFormatStart);
          *highlighting << doFormat;
          textFormatStart = ret.length();
        }
      }

      if( num >= firstDefaultParam ) {
        ret += " = " + *defaultParamIt;
        ++defaultParamIt;
      }

      ++num;
      if( paramNameIt != parameters.end() )
        ++paramNameIt;
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

QVariant CppCodeCompletionModel::getIncludeData(const QModelIndex& index, int role) const
{
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  if( !element || !element->asItem() )
    return QVariant();

  const CompletionItem& completionItem(element->asItem()->item);
  const Cpp::IncludeItem& item( completionItem.includeItem );

  switch (role) {
    case IsExpandable:
      return QVariant(true);
    case ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(item, m_currentTopContext);
      m_navigationWidgets[&completionItem] = nav;

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav);
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

void CppCodeCompletionModel::executeCompletionItem2(Document* document, const Range& word, const QModelIndex& index) const
{
  DUChainReadLocker lock(DUChain::lock());
  
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  if( !element || !element->asItem() )
    return;

  bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
  bool spaceBetweenParens = true;
  bool spaceBetweenEmptyParens = false;
  
  const CompletionItem& item(element->asItem()->item);

  if( index.data( CodeCompletionModel::ArgumentHintDepth ).toInt() )
    return; //Do not replace any text when it is an argument-hint

  QString newText = data(index.sibling(index.row(), Name)).toString();
  int properties = data(index, CompletionRole).toInt();
  
  document->replaceText(word, newText);

  if( properties & CodeCompletionModel::Function ) {
    bool haveArguments = false;
    if( item.declaration && item.declaration->type<CppFunctionType>() && item.declaration->type<CppFunctionType>()->arguments().count() )
      haveArguments = true;
    //Need to have a paren behind
    QString suffix = document->text( KTextEditor::Range( word.end(), word.end() + KTextEditor::Cursor(1, 0) ) );
    if( suffix.trimmed().startsWith("(") ) {
      //Move the cursor behind the opening paren
      if( document->activeView() )
        document->activeView()->setCursorPosition( word.end() + KTextEditor::Cursor( 0, suffix.indexOf('(')+1 ) );
    }else{
      //We need to insert an opening paren
      QString openingParen;
      if( spaceBeforeParen )
        openingParen = " (";
      else
        openingParen = "(";

      if( spaceBetweenParens && (haveArguments || spaceBetweenEmptyParens) )
        openingParen += " ";

      QString closingParen;
      if( spaceBetweenParens && (haveArguments) ) {
        closingParen = " )";
      } else
        closingParen = ")";

      KTextEditor::Cursor jumpPos = word.end() + KTextEditor::Cursor( 0, openingParen.length() );

      //If no arguments, move the cursor behind the closing paren
      if( !haveArguments )
        jumpPos += KTextEditor::Cursor( 0, closingParen.length() );
      
      document->insertText( word.end(), openingParen + closingParen );
      if( document->activeView() )
        document->activeView()->setCursorPosition( jumpPos );
    }
  }
}

QVariant CppCodeCompletionModel::data(const QModelIndex& index, int role) const
{
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  if( !element )
    return QVariant();

  DUChainReadLocker lock(DUChain::lock());

  const CompletionTreeElement& treeElement(*element);

  if( role == CodeCompletionModel::GroupRole ) {
    if( treeElement.asNode() ) {
      return QVariant(treeElement.asNode()->role);
    }else {
      kDebug(9007) << "Requested group-role from leaf tree element";
      return QVariant();
    }
  }else{
    if( treeElement.asNode() ) {
      if( role == treeElement.asNode()->role ) {
        return treeElement.asNode()->roleValue;
      } else {
        kDebug(9007) << "Requested wrong role from non-leaf tree element";
        return QVariant();
      }
    }
  }

  if(!treeElement.asItem()) {
    kWarning(9007) << "Error in completion model";
    return QVariant();
  }

  const CompletionItem& item(treeElement.asItem()->item);

  Declaration* dec = const_cast<Declaration*>( item.declaration.data() );
  if (!dec) {
    if(completionContext()->memberAccessOperation() == Cpp::CodeCompletionContext::IncludeListAccess)
      return getIncludeData(index, role);

    kDebug(9007) <<  "code-completion model: Du-chain item is deleted";
    return QVariant();
  }

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
          Cpp::TypeConversion conv(m_currentTopContext.data());

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
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(DeclarationPointer(dec), m_currentTopContext);
      m_navigationWidgets[&item] = nav;

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav);
       return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case Prefix:
        {
          int depth = item.inheritanceDepth;
          if( depth >= 1000 )
            depth-=1000;
          QString indentation(depth, ' ');

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
      if( item.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
        return QVariant(CustomHighlighting);
      } else {
        return QVariant();
      }
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
      CompletionProperties p = DUChainUtils::completionProperties(dec);

      if (dec->abstractType()) {
        if (CppCVType* cv = dynamic_cast<CppCVType*>(dec->abstractType().data())) {
          if (cv->isConstant())
            p |= Const;
          if (cv->isVolatile())
            ;//TODO
          }

        switch (dec->abstractType()->whichType()) {
          case AbstractType::TypeIntegral:
            if (dec->type<CppEnumerationType>()) {
              // Remove variable bit set in DUChainUtils
              p &= ~Variable;
              p |= Enum;
            }
            break;
          case AbstractType::TypeStructure:
            if (CppClassType::Ptr classType =  dec->type<CppClassType>())
              switch (classType->classType()) {
                case CppClassType::Class:
                  p |= Class;
                  break;
                case CppClassType::Struct:
                  // Remove class bit set in DUChainUtils
                  p &= ~Class;
                  p |= Struct;
                  break;
                case CppClassType::Union:
                  // Remove class bit set in DUChainUtils
                  p &= ~Class;
                  p |= Union;
                  break;
              }
            break;
        }
      }

      if( role == CompletionRole ) {
        return (int)p;

      } else {
        if( index.column() == Icon ) {
          lock.unlock();
          return DUChainUtils::iconForProperties(p);
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
  if( parent.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)parent.internalPointer();

    CompletionTreeNode* node = element->asNode();

    if( !node ) {
      kDebug(9007) << "Requested sub-index of leaf node";
      return QModelIndex();
    }

    if (row < 0 || row >= node->children.count() || column < 0 || column >= ColumnCount)
      return QModelIndex();

    return createIndex(row, column, node->children[row].data());
  } else {
    if (row < 0 || row >= m_completionItems.count() || column < 0 || column >= ColumnCount)
      return QModelIndex();

    return createIndex(row, column, const_cast<CompletionTreeElement*>(m_completionItems[row].data()));
  }
}

QModelIndex CppCodeCompletionModel::parent ( const QModelIndex & index ) const
{
  if( index.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();

    if( element->parent() )
      return createIndex( element->rowInParent(), element->columnInParent(), element->parent() );
  }

  return QModelIndex();
}

int CppCodeCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if( parent.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)parent.internalPointer();

    CompletionTreeNode* node = element->asNode();

    if( !node )
      return 0;

    return node->children.count();
  }else{
    return m_completionItems.count();
  }
}

#include "cppcodecompletionmodel.moc"
