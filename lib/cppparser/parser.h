/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvaluestack.h>
#include <set>
#include <iostream>

struct ParserPrivateData;

class Driver;
class Lexer;
class Token;
struct Error;


class CommentFormatter {
    static inline bool isWhite( QChar c ) {
        return c.isSpace();
    }
    
    static void rStrip( QString str, QString& from ) {
        if( str.isEmpty() ) return;
        
        int i = 0;
        int ip = from.length();
        int s = from.length();
        
        for( int a = s-1; a >= 0; a-- ) {
            if( isWhite( from[a] ) ) {
                continue;
            } else {
                if( from[a] == str[i] ) {
                    i++;
                    ip = a;
                    if( i == (int)str.length() ) break;
                } else {
                    break;
                }
            }
        }
        
        if( ip != (int)from.length() ) from = from.left( ip );
    }
        
    static void strip( QString str, QString& from ) {
        if( str.isEmpty() ) return;
        
        int i = 0;
        int ip = 0;
        int s = from.length();
        
        for( int a = 0; a < s; a++ ) {
            if( isWhite( from[a] ) ) {
                continue;
            } else {
                if( from[a] == str[i] ) {
                    i++;
                    ip = a+1;
                    if( i == (int)str.length() ) break;
                } else {
                    break;
                }
            }
        }
        
        if( ip ) from = from.mid( ip );
    }
    
    public:
    
    static QString formatComment( QString comment ) {
        QString ret;
        int i = 0;
        int s = comment.length();
        while( i < s && comment[i] == '/' ) {
            i++;
        }
        
        if( i > 1 ) {
            ret = comment.mid( i );
        } else {
            ///remove the star in each line
            QStringList lines = QStringList::split( "\n", comment );
            
            if( lines.isEmpty() ) return ret;
            
            strip( "/**", lines.front() );
            rStrip( "/**", lines.back() );
            
            QStringList::iterator it = lines.begin();
            ++it;
            QStringList::iterator eit = lines.end();
            
            if( it != lines.end() ) {
                --eit; 
                
                for( ; it != eit; ++it ) {
                    strip( "*", *it );
                }
                
                if( lines.front().stripWhiteSpace().isEmpty() )
                    lines.pop_front();
            
                if( lines.back().stripWhiteSpace().isEmpty() )
                    lines.pop_back();
            }
            
            ret = lines.join( "\n" );
        }
        
        return ret;
    }
};

class Comment {
    QString m_text;
    int m_line;
    bool m_formatted;
    
    
    void format() {
        if( m_formatted ) return;
        m_formatted = true;
        m_text = CommentFormatter::formatComment( m_text );
    }
    
    public:
        Comment( QString text = "", int line = -1 ) : m_text( text ), m_line( line ), m_formatted(false) {
        }
        
        Comment( int line ) : m_line( line ) {
        }
        
        void operator += ( Comment rhs ) {
            format();
            rhs.format();
            m_text += " " + rhs.m_text;
        }
        
        operator bool() const {
            return !m_text.isEmpty();
        }
        
        operator QString() {
            format();
            return m_text;
        }
        
        inline int line() const {
            return m_line;
        }
        
        bool operator < ( Comment& rhs ) const {
            return m_line < rhs.m_line;
        }
        
        bool isSame ( const Comment& rhs ) {
            if( rhs.m_formatted ) format();
            return m_text == rhs.m_text;
        }
        
        struct cmp {
            bool operator () ( const Comment& c1, const Comment& c2 ) const {
                return c1.line() < c2.line();
            }
        };
};


class CommentStore {
    private:
        typedef std::set< Comment, Comment::cmp > CommentSet;
        CommentSet m_comments;

    public:
        
        ///Returns the comment nearest to "end"(inclusive), and returns & removes it
        Comment getCommentInRange( int end, int start = 0 ) {
            CommentSet::iterator it = m_comments.lower_bound(  end );
            
            
            while( it != m_comments.begin() && (*it).line() > end ) {
                --it;
            }
            
            if( it != m_comments.end() && (*it).line() >= start && (*it).line() <= end ) {
                Comment ret = *it;
                m_comments.erase( it );
                return ret;
            } else {
                return Comment();
            }
        }
        
        ///Returns and removes the comment in the line
        Comment getComment( int line ) {
            CommentSet::iterator it = m_comments.find( line );
            if( it != m_comments.end() ) {
                Comment ret = *it;
                m_comments.erase( it );
                return ret;
            } else {
                return Comment();
            }
        }
        
        void addComment( Comment comment ) {
            
            CommentSet::iterator it = m_comments.find( comment );
            if( it != m_comments.end() ) {
                if( comment.isSame( *it ) ) return;
                Comment c = *it;
                c += comment;
                comment = c;
                m_comments.erase( it );
            }
            
            m_comments.insert( comment );
        }
        
        ///Does not delete the comment
        Comment latestComment() {
            CommentSet::iterator it = m_comments.end(); 
            if( it == m_comments.begin() ) return Comment();
            --it;
            return *it;
        }
        
        void clear() {
            m_comments.clear();
        }
};


class Parser
{
public:
    Parser( Driver* driver, Lexer* lexer );
    virtual ~Parser();

private:
    virtual bool reportError( const Error& err );
    /** @todo remove*/ virtual bool reportError( const QString& msg );
    /** @todo remove*/ virtual void syntaxError();

public /*rules*/ :

    bool parseTranslationUnit( TranslationUnitAST::Node& node );

    bool parseDeclaration( DeclarationAST::Node& node );
    bool parseBlockDeclaration( DeclarationAST::Node& node );
    bool parseLinkageSpecification( DeclarationAST::Node& node );
    bool parseLinkageBody( LinkageBodyAST::Node& node );
    bool parseNamespace( DeclarationAST::Node& node );
    bool parseNamespaceAliasDefinition( DeclarationAST::Node& node );
    bool parseUsing( DeclarationAST::Node& node );
    bool parseUsingDirective( DeclarationAST::Node& node );
    bool parseTypedef( DeclarationAST::Node& node );
    bool parseAsmDefinition( DeclarationAST::Node& node );
    bool parseTemplateDeclaration( DeclarationAST::Node& node );
    bool parseDeclarationInternal( DeclarationAST::Node& node );
    
    bool parseUnqualifiedName( ClassOrNamespaceNameAST::Node& node );
    bool parseStringLiteral( AST::Node& node );
    bool parseName( NameAST::Node& node );
    bool parseOperatorFunctionId( AST::Node& node );
    bool parseTemplateArgumentList( TemplateArgumentListAST::Node& node, bool reportError=true );
    bool parseOperator( AST::Node& node );
    bool parseCvQualify( GroupAST::Node& node );
    bool parseSimpleTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parsePtrOperator( AST::Node& node );
    bool parseTemplateArgument( AST::Node& node );
    bool parseTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parseTypeSpecifierOrClassSpec( TypeSpecifierAST::Node& node );
    bool parseDeclarator( DeclaratorAST::Node& node );
    bool parseTemplateParameterList( TemplateParameterListAST::Node& node );
    bool parseTemplateParameter( TemplateParameterAST::Node& node );
    bool parseStorageClassSpecifier( GroupAST::Node& node );
    bool parseFunctionSpecifier( GroupAST::Node& node );
    bool parseInitDeclaratorList( InitDeclaratorListAST::Node& node );
    bool parseInitDeclarator( InitDeclaratorAST::Node& node );
    bool parseParameterDeclarationClause( ParameterDeclarationClauseAST::Node& node );
    bool parseCtorInitializer( AST::Node& node );
    bool parsePtrToMember( AST::Node& node );
    bool parseEnumSpecifier( TypeSpecifierAST::Node& node );
    bool parseClassSpecifier( TypeSpecifierAST::Node& node );
    bool parseWinDeclSpec( GroupAST::Node& node );
    bool parseElaboratedTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parseDeclaratorId( NameAST::Node& node );
    bool parseExceptionSpecification( GroupAST::Node& node );
    bool parseEnumerator( EnumeratorAST::Node& node );
    bool parseTypeParameter( TypeParameterAST::Node& node );
    bool parseParameterDeclaration( ParameterDeclarationAST::Node& node );
    bool parseTypeId( AST::Node& node );
    bool parseAbstractDeclarator( DeclaratorAST::Node& node );
    bool parseParameterDeclarationList( ParameterDeclarationListAST::Node& node );
    bool parseMemberSpecification( DeclarationAST::Node& node );
    bool parseAccessSpecifier( AST::Node& node );
    bool parseTypeIdList( GroupAST::Node& node );
    bool parseMemInitializerList( AST::Node& node );
    bool parseMemInitializer( AST::Node& node );
    bool parseInitializer( AST::Node& node );
    bool parseBaseClause( BaseClauseAST::Node& node );
    bool parseBaseSpecifier( BaseSpecifierAST::Node& node );
    bool parseInitializerClause( AST::Node& node );
    bool parseMemInitializerId( NameAST::Node& node );
    bool parseFunctionBody( StatementListAST::Node& node );

    // expression
    bool skipExpression( AST::Node& node );
    bool skipCommaExpression( AST::Node& node );
    bool skipExpressionStatement( StatementAST::Node& node );

    bool parseExpression( AST::Node& node );
    bool parsePrimaryExpression( AST::Node& node );
    bool parsePostfixExpression( AST::Node& node );
    bool parseUnaryExpression( AST::Node& node );
    bool parseNewExpression( AST::Node& node );
    bool parseNewTypeId( AST::Node& node );
    bool parseNewDeclarator( AST::Node& node );
    bool parseNewInitializer( AST::Node& node );
    bool parseDeleteExpression( AST::Node& node );
    bool parseCastExpression( AST::Node& node );
    bool parsePmExpression( AST::Node& node );
    bool parseMultiplicativeExpression( AST::Node& node );
    bool parseAdditiveExpression( AST::Node& node );
    bool parseShiftExpression( AST::Node& node );
    bool parseRelationalExpression( AST::Node& node, bool templArgs=false );
    bool parseEqualityExpression( AST::Node& node, bool templArgs=false );
    bool parseAndExpression( AST::Node& node, bool templArgs=false );
    bool parseExclusiveOrExpression( AST::Node& node, bool templArgs=false );
    bool parseInclusiveOrExpression( AST::Node& node, bool templArgs=false );
    bool parseLogicalAndExpression( AST::Node& node, bool templArgs=false );
    bool parseLogicalOrExpression( AST::Node& node, bool templArgs=false );
    bool parseConditionalExpression( AST::Node& node );
    bool parseAssignmentExpression( AST::Node& node );
    bool parseConstantExpression( AST::Node& node );
    bool parseCommaExpression( AST::Node& node );
    bool parseThrowExpression( AST::Node& node );

    // statement
    bool parseCondition( ConditionAST::Node& node );
    bool parseStatement( StatementAST::Node& node );
    bool parseWhileStatement( StatementAST::Node& node );
    bool parseDoStatement( StatementAST::Node& node );
    bool parseForStatement( StatementAST::Node& node );
    bool parseForEachStatement( StatementAST::Node& node ); // qt4 [erbsland]
    bool parseCompoundStatement( StatementAST::Node& node );
    bool parseForInitStatement( StatementAST::Node& node );
    bool parseIfStatement( StatementAST::Node& node );
    bool parseSwitchStatement( StatementAST::Node& node );
    bool parseLabeledStatement( StatementAST::Node& node );
    bool parseDeclarationStatement( StatementAST::Node& node );
    bool parseTryBlockStatement( StatementAST::Node& node );
    
    // objective c
    bool parseObjcDef( DeclarationAST::Node& node );
    bool parseObjcClassDef( DeclarationAST::Node& node );
    bool parseObjcClassDecl( DeclarationAST::Node& node );
    bool parseObjcProtocolDecl( DeclarationAST::Node& node );
    bool parseObjcAliasDecl( DeclarationAST::Node& node );
    bool parseObjcProtocolDef( DeclarationAST::Node& node );
    bool parseObjcMethodDef( DeclarationAST::Node& node );
    
    bool parseIvarDeclList( AST::Node& node ); 
    bool parseIvarDecls( AST::Node& node ); 
    bool parseIvarDecl( AST::Node& node ); 
    bool parseIvars( AST::Node& node ); 
    bool parseIvarDeclarator( AST::Node& node );
    bool parseMethodDecl( AST::Node& node ); 
    bool parseUnarySelector( AST::Node& node ); 
    bool parseKeywordSelector( AST::Node& node ); 
    bool parseSelector( AST::Node& node );
    bool parseKeywordDecl( AST::Node& node ); 
    bool parseReceiver( AST::Node& node ); 
    bool parseObjcMessageExpr( AST::Node& node ); 
    bool parseMessageArgs( AST::Node& node );
    bool parseKeywordExpr( AST::Node& node ); 
    bool parseKeywordArgList( AST::Node& node ); 
    bool parseKeywordArg( AST::Node& node ); 
    bool parseReservedWord( AST::Node& node );
    bool parseMyParms( AST::Node& node ); 
    bool parseMyParm( AST::Node& node ); 
    bool parseOptParmList( AST::Node& node ); 
    bool parseObjcSelectorExpr( AST::Node& node );
    bool parseSelectorArg( AST::Node& node ); 
    bool parseKeywordNameList( AST::Node& node ); 
    bool parseKeywordName( AST::Node& node ); 
    bool parseObjcEncodeExpr( AST::Node& node );
    bool parseObjcString( AST::Node& node ); 
    bool parseProtocolRefs( AST::Node& node ); 
    bool parseIdentifierList( GroupAST::Node& node ); 
    bool parseIdentifierColon( AST::Node& node );
    bool parseObjcProtocolExpr( AST::Node& node ); 
    bool parseObjcOpenBracketExpr( AST::Node& node ); 
    bool parseObjcCloseBracket( AST::Node& node );
  
    void nextToken( bool skipComments = true );
    
    ///parses all comments until the end of the line
    Comment comment();
    void preparseLineComments( int line );
    void processComment( int offset = 0 );
    void clearComment( );
    
    bool skipUntil( int token );
    bool skipUntilDeclaration();
    bool skipUntilStatement();
    bool skip( int l, int r );
    QString toString( int start, int end, const QString& sep=" " ) const;

private:
    int currentLine();
    CommentStore m_commentStore;
    
    template<class Type>
    void eventuallyTakeComment( int startLn, int line, Type& ast );
    template<class Type>
    void eventuallyTakeComment( Type& ast );
    
    ParserPrivateData* d;
    Driver* m_driver;
    Lexer* lex;
    Comment m_currentComment;
    int m_problems;
    int m_maxProblems;
    bool objcp;

private:
    Parser( const Parser& source );
    void operator = ( const Parser& source );
};


#endif
