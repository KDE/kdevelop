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

#include "helpers.h"
#include "item.h"
#include <QList>
#include <QVariant>
#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/duchainutils.h>
#include <QTextFormat>
#include <QStringList>
#include "../cppduchain/overloadresolution.h"
#include "../cppduchain/cpptypes.h"
#include "../cppduchain/cppduchain.h"
#include <templatedeclaration.h>
#include <templateparameterdeclaration.h>
#include "../cpputils.h"
#include "context.h"

const int desiredArgumentTypeLength = 20;
const int maxDefaultParameterLength = 30;

using namespace KDevelop;

namespace Cpp {

void createArgumentList(const NormalDeclarationCompletionItem& item, QString& ret, QList<QVariant>* highlighting, bool includeDefaultParams, bool noShortening )
{
  Declaration* dec(item.declaration().data());

  Cpp::CodeCompletionContext::Function f;
  DUContext* ctx = 0;
  if(item.completionContext() && item.completionContext()->duContext()) {
    ctx = item.completionContext()->duContext();
  }

  if( item.completionContext() && item.completionContext()->accessType() == Cpp::CodeCompletionContext::FunctionCallAccess && item.completionContext()->functions().count() > item.listOffset )
    f = item.completionContext()->functions()[item.listOffset];

  QTextFormat normalFormat(QTextFormat::CharFormat);
  QTextFormat highlightFormat; //highlightFormat is invalid, so kate uses the match-quality dependent color.
  QTextCharFormat boldFormat;
  boldFormat.setFontWeight(QFont::Bold);

  AbstractFunctionDeclaration* decl = dynamic_cast<AbstractFunctionDeclaration*>(dec);
  FunctionType::Ptr functionType = dec->type<FunctionType>();
  if (functionType && decl) {

    QVector<Declaration*> parameters;
    if( DUChainUtils::getArgumentContext(dec) )
      parameters = DUChainUtils::getArgumentContext(dec)->localDeclarations(ctx ? ctx->topContext() : 0);

//     QStringList defaultParams = decl->defaultParameters();

    QVector<Declaration*>::const_iterator paramNameIt = parameters.constBegin();
    uint defaultParamNum = 0;

    int firstDefaultParam = functionType->arguments().count() - decl->defaultParametersSize();

    int textFormatStart = ret.length();
    ret += "(";
    
    if( highlighting && ret.length() != textFormatStart )
    {
      //Add a default-highlighting for the passed text
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length() - textFormatStart);
      *highlighting << boldFormat;
      textFormatStart = ret.length();
    }
    
    bool first = true;
    int num = 0;

    const KDevVarLengthArray<Cpp::ViableFunction::ParameterConversion>& conversions = f.function.parameterConversions();
    int parameterConversion = 0;

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
        QTextCharFormat format;

        if( parameterConversion != conversions.size() ) {
          //Interpolate the color
          quint64 badMatchColor = 0xff7777ff; //Full blue
          quint64 goodMatchColor = 0xff77ff77; //Full green

          uint totalColor = (badMatchColor*(Cpp::MaximumConversionResult-conversions[parameterConversion].rank) + goodMatchColor*(conversions[parameterConversion]).rank)/Cpp::MaximumConversionResult;

          format.setUnderlineStyle( QTextCharFormat::WaveUnderline );
          format.setUnderlineColor( QColor(totalColor) );

          ++parameterConversion;
          doFormat = format;
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

      if( paramNameIt != parameters.constEnd() /*&& !(*paramNameIt)->identifier().isEmpty()*/ ) {
        // make sure that something like 'int foo[][1][2]' gets printed as such
        // and not like 'int[0][1][2] foo'
        AbstractType::Ptr type;
        if(ctx)
          type = typeForShortenedString(*paramNameIt);
        else
          type = argument;

        QString arrayAppendix;
        ArrayType::Ptr arrayType;
        while ((arrayType = type.cast<ArrayType>())) {
          type = arrayType->elementType();
          //note: we have to prepend since we iterate from outside, i.e. from right to left.
          if (arrayType->dimension()) {
            arrayAppendix.prepend(QString("[%1]").arg(arrayType->dimension()));
          } else {
            // dimensionless
            arrayAppendix.prepend("[]");
          }
        }

        if(noShortening) {
          ret += Cpp::shortenedTypeString(type, ctx, 1000000);
        }else
          ret += Cpp::shortenedTypeString(type, ctx, desiredArgumentTypeLength, item.stripPrefix());
        const QString ident = (*paramNameIt)->identifier().toString();
        if (!ident.isEmpty()) {
          ret += " " + ident;
        }
        ret += arrayAppendix;
      } else if (argument)
        ret += argument->toString();
      else
        ret += "<incomplete type>";

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

      if( num >= firstDefaultParam && includeDefaultParams ) {
        ret += " = ";
        
        QString defaultParam = decl->defaultParameters()[defaultParamNum].str();
        if(defaultParam.length() <= maxDefaultParameterLength)
          ret += defaultParam;
        else
          ret += "...";
        
        ++defaultParamNum;
      }

      ++num;
      if( paramNameIt != parameters.constEnd() )
        ++paramNameIt;
    }

    if( highlighting && ret.length() != textFormatStart )
    {
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length() - textFormatStart);
      *highlighting << normalFormat;
      textFormatStart = ret.length();
    }
    
    ret += ')';

    if( highlighting && ret.length() != textFormatStart )
    {
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length() - textFormatStart);
      *highlighting << boldFormat;
      textFormatStart = ret.length();
    }

    return;
  }
}

void createTemplateArgumentList(const NormalDeclarationCompletionItem& item, QString& ret, QList<QVariant>* highlighting, bool includeDefaultParams )
{
  Q_UNUSED(includeDefaultParams);

  Declaration* dec(item.declaration().data());

  QTextFormat normalFormat(QTextFormat::CharFormat);
  QTextFormat highlightFormat; //highlightFormat is invalid, so kate uses the match-quality dependent color.
  QTextCharFormat boldFormat;
  boldFormat.setFontWeight(QFont::Bold);

  TemplateDeclaration* tempDecl(dynamic_cast<TemplateDeclaration*>(dec));
  if(!tempDecl)
    return;
  
  DUContext* templateContext = tempDecl->templateContext( dec->topContext() );
  
  if(!templateContext)
    return;
  
  QVector<Declaration*> parameters = templateContext->localDeclarations();

  int textFormatStart = ret.length();
  ret += "<";
  
  if( highlighting && ret.length() != textFormatStart )
  {
    //Add a default-highlighting for the passed text
    *highlighting <<  QVariant(textFormatStart);
    *highlighting << QVariant(ret.length() - textFormatStart);
    *highlighting << boldFormat;
    textFormatStart = ret.length();
  }
  
  bool first = true;
  int num = 0;
  foreach(Declaration* decl, parameters) 
  {
    if (first)
      first = false;
    else
      ret += ", ";
    
    ///@todo Find out whether it's class or another type
    if(decl->abstractType() && !decl->type<CppTemplateParameterType>())
      ret += decl->abstractType()->toString();
    else
      ret += "class";
    
    ret += " ";
    
    if( highlighting && ret.length() != textFormatStart )
    {
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length() - textFormatStart);
      *highlighting << normalFormat;
      textFormatStart = ret.length();
    }
    
    ret += decl->identifier().toString();

    if( highlighting && ret.length() != textFormatStart )
    {
      *highlighting <<  QVariant(textFormatStart);
      *highlighting << QVariant(ret.length() - textFormatStart);
      if(item.completionContext() && item.completionContext()->accessType() == Cpp::CodeCompletionContext::TemplateAccess && num == item.completionContext()->matchPosition())
        *highlighting << highlightFormat;
      else
        *highlighting << normalFormat;
      textFormatStart = ret.length();
    }
    
    TemplateParameterDeclaration* paramDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
    if(!paramDecl->defaultParameter().isEmpty())
      ret += " = " + paramDecl->defaultParameter().toString();
    
    ++num;
  }
  
  if( highlighting && ret.length() != textFormatStart )
  {
    *highlighting <<  QVariant(textFormatStart);
    *highlighting << QVariant(ret.length() - textFormatStart);
    *highlighting << normalFormat;
    textFormatStart = ret.length();
  }

  ret += '>';

  if( highlighting && ret.length() != textFormatStart )
  {
    *highlighting <<  QVariant(textFormatStart);
    *highlighting << QVariant(ret.length() - textFormatStart);
    *highlighting << boldFormat;
    textFormatStart = ret.length();
  }

  return;
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

bool isSource(QString file) {
  foreach(const QString& ext, sourceExtensions)
    if(file.endsWith(ext))
      return true;
  return false;
}

}
