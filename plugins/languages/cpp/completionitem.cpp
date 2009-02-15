/*
 * KDevelop C++ Code Completion Support
 *
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

#include "completionitem.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <cpptypes.h>
#include <QModelIndex>
#include "completionhelpers.h"
#include "cppcodecompletionmodel.h"
#include <language/duchain/declaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include "cppduchain/navigation/navigationwidget.h"
#include <language/duchain/duchainutils.h>
#include <classdeclaration.h>
#include "cppduchain/qtfunctiondeclaration.h"
#include <language/duchain/use.h>
#include <typeutils.h>
#include <cppduchain.h>
#include <templatedeclaration.h>

using namespace KDevelop;

///@todo Implement a proper duchain based shortening-scheme, and use it throughout the completion
//If this is true, the return-values of argument-hints will be just written as "..." if they are too long
const bool shortenArgumentHintReturnValues = true;
const int maximumArgumentHintReturnValueLength = 30;
const int desiredTypeLength = 20;
const int normalBestMatchesCount = 5;

void NormalDeclarationCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  //We have to use word directly, because it may be a smart-range that is updated during insertions and such
  bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
  bool spaceBetweenParens = false;
  bool spaceBetweenEmptyParens = false;

  if( completionContext && completionContext->depth() != 0 )
    return; //Do not replace any text when it is an argument-hint

  if(m_isQtSignalSlotCompletion) {
    bool addSignalSlot = true;
    {
      //Check whether we need to add SLOT( or SIGNAL(
      QString prefixText = document->text(KTextEditor::Range(word.start().line(), 0, word.start().line(), word.start().column()));
      kDebug() << "prefix" << prefixText;
      QRegExp signalRegExp("SIGNAL(\\s)*\\(");
      QRegExp slotRegExp("SLOT(\\s)*\\(");
      int signalAt = signalRegExp.lastIndexIn(prefixText);
      kDebug() << "signalRegExp found at" << signalAt;
      if(signalAt != -1 && prefixText.mid(signalAt + signalRegExp.matchedLength()).trimmed().isEmpty())
        addSignalSlot = false;
      int slotAt = slotRegExp.lastIndexIn(prefixText);
      if(slotAt != -1 && prefixText.mid(slotAt + slotRegExp.matchedLength()).trimmed().isEmpty())
        addSignalSlot = false;
    }
    
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    QString functionSignature;
    Cpp::QtFunctionDeclaration* classFun = dynamic_cast<Cpp::QtFunctionDeclaration*>(m_declaration.data());
    if(classFun && classFun->type<FunctionType>() && (classFun->isSignal() || classFun->isSlot())) {
      ///@todo Replace previous signal/slot specifications
      functionSignature = classFun->identifier().toString();
      functionSignature += '(' + classFun->normalizedSignature().str() + ')';
      if(addSignalSlot) {
        if(classFun->isSignal())
          functionSignature = "SIGNAL(" + functionSignature + ')';
        else
          functionSignature = "SLOT(" + functionSignature + ')';
      }else{
        //Only add a closing )
        functionSignature += ')';
      }
    }
    lock.unlock();
    document->replaceText(word, functionSignature);
    return;
  }
  
  QString newText;

  if(!useAlternativeText) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(m_declaration) {
      newText = m_declaration->identifier().toString();
    } else {
      kDebug() << "Declaration disappeared";
      return;
    }
  }else{
    newText = alternativeText;
  }

  document->replaceText(word, newText);
  
  bool jumpForbidden = false;
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  Cpp::TemplateDeclaration* templateDecl = dynamic_cast<Cpp::TemplateDeclaration*>(m_declaration.data());
  if(templateDecl) {
    DUContext* context = templateDecl->templateContext(m_declaration->topContext());
    if(context && context->localDeclarations().count() && context->localDeclarations()[0]->type<CppTemplateParameterType>()) {
      jumpForbidden = true;
      lock.unlock();
      document->insertText( word.end(), "<>" );
      document->activeView()->setCursorPosition( word.end() - KTextEditor::Cursor(0, 1) );
      lock.lock();
    }
  }
  
  if(m_declaration.data()->kind() == Declaration::Namespace) {
    lock.unlock();
    document->insertText(word.end(), "::");
    lock.lock();
  }
    

  if( !useAlternativeText && m_declaration && (dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data()) /*|| m_declaration->kind() == Declaration::Type*/) ) { //Do some intelligent stuff for functions with the parens:
    bool haveArguments = false;
    if( m_declaration->kind() == Declaration::Type || (m_declaration->type<FunctionType>() && m_declaration->type<FunctionType>()->indexedArgumentsSize()) )
      haveArguments = true;
    //Need to have a paren behind
    QString suffix = document->text( KTextEditor::Range( word.end(), word.end() + KTextEditor::Cursor(1, 0) ) );
    if( suffix.trimmed().startsWith('(') ) {
      //Move the cursor behind the opening paren
      if( document->activeView() )
        document->activeView()->setCursorPosition( word.end() + KTextEditor::Cursor( 0, suffix.indexOf('(')+1 ) );
    }else{
      //We need to insert an opening paren
      QString openingParen;
      if( spaceBeforeParen )
        openingParen = " (";
      else
        openingParen = '(';

      if( spaceBetweenParens && (haveArguments || spaceBetweenEmptyParens) )
        openingParen += ' ';

      QString closingParen;
      if( spaceBetweenParens && (haveArguments) ) {
        closingParen = " )";
      } else
        closingParen = ')';

      KTextEditor::Cursor jumpPos = word.end() + KTextEditor::Cursor( 0, openingParen.length() );

      //If no arguments, move the cursor behind the closing paren
      if( !haveArguments )
        jumpPos += KTextEditor::Cursor( 0, closingParen.length() );

      lock.unlock();
      document->insertText( word.end(), openingParen + closingParen );
      if(!jumpForbidden) {
        if( document->activeView() )
          document->activeView()->setCursorPosition( jumpPos );
      }
    }
  }
}

const bool indentByDepth = false;

//The name to be viewed in the name column
inline QString nameForDeclaration(Declaration* dec) {
  QString ret = dec->identifier().toString();

  if (ret.isEmpty())
    return "<unknown>";
  else
    return ret;
}

KTextEditor::CodeCompletionModel::CompletionProperties NormalDeclarationCompletionItem::completionProperties() const {
  Declaration* dec = m_declaration.data();
  if(!dec)
    return (KTextEditor::CodeCompletionModel::CompletionProperties)0;

  CodeCompletionModel::CompletionProperties p = DUChainUtils::completionProperties(dec);

  AbstractType::Ptr type(dec->abstractType());
  if (type) {
    if (type->modifiers() & AbstractType::ConstModifier)
      p |= CodeCompletionModel::Const;
    if (type->modifiers() & AbstractType::VolatileModifier)
      ;//TODO

    switch (dec->abstractType()->whichType()) {
      case AbstractType::TypeIntegral:
        if (dec->type<EnumerationType>()) {
          // Remove variable bit set in DUChainUtils
          p &= ~CodeCompletionModel::Variable;
          p |= CodeCompletionModel::Enum;
        }
        if (dec->type<EnumeratorType>()) {
          //Get the properties from the parent, because that may contain information like "private"
          if(dec->context()->owner()) {
            p = DUChainUtils::completionProperties(dec->context()->owner());
          }
          // Remove variable bit set in DUChainUtils
          p &= 0xffffffff - CodeCompletionModel::Variable;
          p |= CodeCompletionModel::Enum;
        }
        break;
      case AbstractType::TypeStructure:
        if (CppClassType::Ptr classType =  dec->type<CppClassType>())
          p |= CodeCompletionModel::Class;
        break;
    }
  }

  if(useAlternativeText) {
    //Remove other scope flags, and add the local scope flag
    ///@todo Create an own "Hint" scope within KTextEditor::CodeCompletionModel, and use that
    p &= ~CodeCompletionModel::GlobalScope;
    p &= ~CodeCompletionModel::NamespaceScope;
    p |= CodeCompletionModel::LocalScope;
  }
  return p;
}

bool declarationNeedsTemplateParameters(Declaration* decl) {
  Cpp::TemplateDeclaration* asTemplate = dynamic_cast<Cpp::TemplateDeclaration*>(decl);
  if(asTemplate) {
    DUContext* templateContext = asTemplate->templateContext(decl->topContext());
    if(templateContext) {
      foreach(Declaration* decl, templateContext->localDeclarations()) {
        if(decl->type<CppTemplateParameterType>())
          return true;
      }
    }
  }
  return false;
}

QList<KDevelop::IndexedType> NormalDeclarationCompletionItem::typeForArgumentMatching() const {
  QList<KDevelop::IndexedType> ret;
  if( m_declaration && completionContext && completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && listOffset < completionContext->functions().count() )
  {
    Cpp::CodeCompletionContext::Function f( completionContext->functions()[listOffset] );

    if( f.function.isValid() && f.function.isViable() && f.function.declaration() && f.function.declaration()->type<FunctionType>() && f.function.declaration()->type<FunctionType>()->indexedArgumentsSize() > f.matchedArguments ) {
      ret << f.function.declaration()->type<FunctionType>()->indexedArguments()[f.matchedArguments];
    }
  }
  return ret;
}

CompletionTreeItemPointer currentMatchContext;

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const {

  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }

  //Stuff that does not require a declaration:
  switch (role) {
    case CodeCompletionModel::SetMatchContext:
      currentMatchContext = CompletionTreeItemPointer(const_cast<NormalDeclarationCompletionItem*>(this));
      return QVariant(1);
  };

  if(!m_declaration) {
    if(role == Qt::DisplayRole && index.column() == CodeCompletionModel::Name)
      return alternativeText;
    return QVariant();
  }else if(useAlternativeText) {
    if(role == Qt::DisplayRole) {
      if(index.column() == CodeCompletionModel::Name)
        return alternativeText;
      else
        return QVariant();
    }
  }

  Declaration* dec = const_cast<Declaration*>( m_declaration.data() );

  switch (role) {
    case CodeCompletionModel::BestMatchesCount:
      return QVariant(normalBestMatchesCount);
    break;
    case CodeCompletionModel::MatchQuality:
    {
      if(m_fixedMatchQuality != -1)
        return QVariant(m_fixedMatchQuality);
      
      if( currentMatchContext && currentMatchContext->typeForArgumentMatching().size()) {
        
        int bestQuality = 0;
        foreach(IndexedType type, currentMatchContext->typeForArgumentMatching()) {
        
        Cpp::TypeConversion conv(model->currentTopContext().data());
 
        AbstractType::Ptr ownType = effectiveType(dec);
        
        bool fromLValue = (bool)ownType.cast<ReferenceType>() || (!dynamic_cast<AbstractFunctionDeclaration*>(dec) && dec->kind() == Declaration::Instance);
        
        ///@todo fill the lvalue-ness correctly
        int q = ( conv.implicitConversion( completionContext->applyPointerConversionForMatching(ownType->indexed()), type, fromLValue )  * 10 ) / Cpp::MaximumConversionResult;
        if(q > bestQuality)
          bestQuality = q;
        }
        
        return QVariant(bestQuality);
      }
    }
    return QVariant();
    case CodeCompletionModel::ItemSelected:
       return QVariant(Cpp::NavigationWidget::shortDescription(dec));
    case CodeCompletionModel::IsExpandable:
      return QVariant(true);
    case CodeCompletionModel::ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(DeclarationPointer(dec), model->currentTopContext());
      model->addNavigationWidget(this, nav);

      QVariant v;
      v.setValue<QWidget*>((QWidget*)nav);
      return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
        {
          if(m_isQtSignalSlotCompletion) {
            Cpp::QtFunctionDeclaration* funDecl = dynamic_cast<Cpp::QtFunctionDeclaration*>(dec);
            if(funDecl) {
              if(funDecl->isSignal())
                return QVariant("SIGNAL");
              if(funDecl->isSlot())
                return QVariant("SLOT");
            }
          }
          int depth = m_inheritanceDepth;
          if( depth >= 1000 )
            depth-=1000;
          QString indentation;
          if(indentByDepth)
            indentation = QString(depth, ' ');

          if(m_declaration->kind() == Declaration::Namespace)
            return indentation + "namespace";
          
          if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(dec) ) {
            if( alias->identifier().isEmpty() ) {
              return indentation + "using namespace";/* " + alias->importIdentifier().toString();*/
            } else {
              return indentation + "namespace";/* " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();*/
            }
          }

          if( dec->isTypeAlias() )
            indentation += "typedef ";

          if( dec->kind() == Declaration::Type && !dec->type<FunctionType>() && !dec->isTypeAlias() ) {
              if (CppClassType::Ptr classType =  dec->type<CppClassType>()){
                Cpp::ClassDeclaration* classDecl  = dynamic_cast<Cpp::ClassDeclaration*>(dec);
                if(classDecl) {
                  switch (classDecl->classType()) {
                    case Cpp::ClassDeclarationData::Class:
                      return indentation + "class";
                      break;
                    case Cpp::ClassDeclarationData::Struct:
                      return indentation + "struct";
                      break;
                    case Cpp::ClassDeclarationData::Union:
                      return indentation + "union";
                      break;
                  }
                }else if(dec->isForwardDeclaration()) {
                  return indentation + "class"; ///@todo Would be useful to have the class/struct/union info also for forward-declarations
                }
              }
              if(dec->type<EnumerationType>()) {
                return "enum";
              }
            return QVariant();
          }
          if (dec->abstractType()) {
            if(EnumeratorType::Ptr enumerator = dec->type<EnumeratorType>()) {
              if(dec->context()->owner() && dec->context()->owner()->abstractType()) {
                if(!dec->context()->owner()->identifier().isEmpty())
                  return dec->context()->owner()->abstractType()->toString();
                else
                  return "enum";
              }
            }
            if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
              ClassFunctionDeclaration* funDecl = dynamic_cast<ClassFunctionDeclaration*>(dec);

              if (functionType->returnType()) {
                QString ret = indentation + Cpp::shortenedTypeString(dec, desiredTypeLength);
                if(shortenArgumentHintReturnValues && argumentHintDepth() && ret.length() > maximumArgumentHintReturnValueLength)
                  return QString("...");
                else
                  return ret;
              }else if(argumentHintDepth()) {
                return indentation;//Don't show useless prefixes in the argument-hints
              }else if(funDecl && funDecl->isConstructor() )
                return indentation + "<constructor>";
              else if(funDecl && funDecl->isDestructor() )
                return indentation + "<destructor>";
              else
                return indentation + "<incomplete type>";

            } else {
              QString ret = indentation;
              return  ret + Cpp::shortenedTypeString(dec, desiredTypeLength);
            }
          } else {
            return indentation + "<incomplete type>";
          }
        }

        case CodeCompletionModel::Scope: {
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

        case CodeCompletionModel::Name:
          return nameForDeclaration(dec);

        case CodeCompletionModel::Arguments:
        {
          QString ret;
          
          if(m_isTemplateCompletion || declarationNeedsTemplateParameters(dec))
            createTemplateArgumentList(*this, ret, 0);
          
          if (dec->type<FunctionType>())
            createArgumentList(*this, ret, 0);
          
          return ret;
        }
        break;
        case CodeCompletionModel::Postfix:
          if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
            // Retrieve const/volatile string
            return functionType->AbstractType::toString();
          }
          break;
      }
      break;
    case CodeCompletionModel::HighlightingMethod:
    if( index.column() == CodeCompletionModel::Arguments ) {
//       if( completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
        return QVariant(CodeCompletionModel::CustomHighlighting);
/*      }else{
        return QVariant();
      }
      break;*/
    } /*else if(index.column() == CodeCompletionModel::Name) {
      return QVariant(CodeCompletionModel::CustomHighlighting);
    }*/

    break;

    case CodeCompletionModel::CustomHighlight:
    if( index.column() == CodeCompletionModel::Arguments /*&& completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess*/ ) {
      QString ret;
      QList<QVariant> highlight;
      if(m_isTemplateCompletion || declarationNeedsTemplateParameters(dec))
        createTemplateArgumentList(*this, ret, &highlight);
      
      if (dec->type<FunctionType>())
        createArgumentList(*this, ret, &highlight);
      return QVariant(highlight);
    }
//     if( index.column() == CodeCompletionModel::Name ) {
//       //Bold
//       QTextCharFormat boldFormat;
//       boldFormat.setFontWeight((QFont::Normal + QFont::DemiBold)/2);
// 
//       QList<QVariant> ret;
//       ret << 0;
//       ret << nameForDeclaration(dec).length();
//       ret << QVariant(boldFormat);
// 
//       return QVariant(ret);
//     }
    break;
    case CodeCompletionModel::CompletionRole:
      return (int)completionProperties();
    case Qt::DecorationRole:
     {
      CodeCompletionModel::CompletionProperties p = completionProperties();

      //If it's a signal, remove t he protected flag when computing the decoration. Signals are always protected, and this will give a nicer icon.
      if(p & CodeCompletionModel::Signal)
        p = CodeCompletionModel::Signal;
      //If it's a slot, remove all flags except the slot flag, because that will give a nicer icon. Access-rights are checked anyway.
      if(p & CodeCompletionModel::Slot)
        p = CodeCompletionModel::Slot;
      
      
      if( index.column() == CodeCompletionModel::Icon ) {
        lock.unlock();
        return DUChainUtils::iconForProperties(p);
      }
      break;
    }

    case CodeCompletionModel::ScopeIndex:
      return static_cast<int>(reinterpret_cast<long>(dec->context()));
  }

  return QVariant();
}

int NormalDeclarationCompletionItem::inheritanceDepth() const
{
  return m_inheritanceDepth;
}

int IncludeFileCompletionItem::inheritanceDepth() const
{
  return includeItem.pathNumber;
}

int NormalDeclarationCompletionItem::argumentHintDepth() const
{
  if( completionContext )
      return completionContext->depth();
    else
      return 0;
}

int IncludeFileCompletionItem::argumentHintDepth() const
{
  return 0;
}

QVariant IncludeFileCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }

  const Cpp::IncludeItem& item( includeItem );

  switch (role) {
    case CodeCompletionModel::IsExpandable:
      return QVariant(true);
    case CodeCompletionModel::ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(item, model->currentTopContext());
      model->addNavigationWidget(this, nav);

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav);
       return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
          if(item.isDirectory)
            return QVariant("directory");
          else
            return QVariant("file");
        case CodeCompletionModel::Name: {
          return item.isDirectory ? item.name + '/' : item.name;
        }
      }
      break;
    case CodeCompletionModel::ItemSelected:
    {
      return QVariant( Cpp::NavigationWidget::shortDescription(item) );
    }
  }

  return QVariant();
}

void IncludeFileCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& _word) {

  KTextEditor::Range word(_word);

  QString newText = includeItem.isDirectory ? includeItem.name + '/' : includeItem.name;

  if(!includeItem.isDirectory) {
    //Add suffix and newline
    QString lineText = document->line( word.end().line() ).trimmed();
    if(lineText.startsWith("#include")) {
      lineText = lineText.mid(8).trimmed();
      if(lineText.startsWith('"'))
        newText += '\"';
      else if(lineText.startsWith('<'))
        newText += '>';
    }

    word.end().setColumn( document->lineLength(word.end().line()) );
  }

  document->replaceText(word, newText);
}

void TypeConversionCompletionItem::setPrefix(QString s) {
  m_prefix = s;
}

QList<KDevelop::IndexedType> TypeConversionCompletionItem::typeForArgumentMatching() const {
  return QList<KDevelop::IndexedType>() << m_type;
}

QList<KDevelop::IndexedType> TypeConversionCompletionItem::type() const {
  return QList<KDevelop::IndexedType>() << m_type;
}

void TypeConversionCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  if(argumentHintDepth() == 0)
    document->replaceText( word, m_text );
}

QVariant TypeConversionCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {

  switch (role) {
    case CodeCompletionModel::SetMatchContext:
      currentMatchContext = CompletionTreeItemPointer(const_cast<TypeConversionCompletionItem*>(this));
      return QVariant(1);
    case CodeCompletionModel::BestMatchesCount:
      return QVariant(normalBestMatchesCount);
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
          return QVariant(m_prefix);
        case CodeCompletionModel::Name: {
          return m_text;
        }
      }
      break;
    case CodeCompletionModel::ItemSelected:
      return QVariant();
    case CodeCompletionModel::MatchQuality:
    {
      DUChainReadLocker lock(DUChain::lock(), 500);
      if(!lock.locked()) {
        kDebug(9007) << "Failed to lock the du-chain in time";
        return QVariant();
      }
      
      if( currentMatchContext && currentMatchContext->typeForArgumentMatching().size() ) {
        
          int bestQuality = 0;
          
          foreach(IndexedType type, currentMatchContext->typeForArgumentMatching()) {
          Cpp::TypeConversion conv(model->currentTopContext().data());

          ///@todo Think about lvalue-ness
          foreach(IndexedType ownType, typeForArgumentMatching()) {
            int quality = ( conv.implicitConversion( completionContext->applyPointerConversionForMatching(ownType), type, false )  * 10 ) / Cpp::MaximumConversionResult;
            if(quality > bestQuality)
              bestQuality = quality;
          }
        }
        
        return QVariant(bestQuality);
      }
    }
    return QVariant();
  }

  return QVariant();
}

int TypeConversionCompletionItem::argumentHintDepth() const {
  return m_argumentHintDepth;
}

TypeConversionCompletionItem::TypeConversionCompletionItem(QString text, KDevelop::IndexedType type, int argumentHintDepth, KSharedPtr<Cpp::CodeCompletionContext> _completionContext) : m_text(text), m_type(type), m_argumentHintDepth(argumentHintDepth), completionContext(_completionContext) {
}
