#include "/media/data/kdedev/4.0/kdevplatform/src/language/duchain/duchain.h" /* defines DUChain */
#include "languages/cpp/duchainbuilder/usebuilder.h" /* defines UseBuilder */
#include "languages/cpp/duchainbuilder/declarationbuilder.h" /* defines DeclarationBuilder */
#include "languages/cpp/duchainbuilder/dumpchain.h" /* defines DumpChain */
#include "ducontext.h"
#include "expressionparser.h"
#include "ast.h"
#include "parsesession.h"
#include "parser.h"
#include "control.h"
#include "duchainbuilder/dumptypes.h" /* defines DumpTypes */
#include "duchainlock.h"
//#include "typerepository.h"
#include <identifier.h>


namespace KDevelop {
ExpressionEvaluationResult::Pointer ExpressionParser::evaluateType( const QByteArray& unit, DUContext* context, bool debug ) {

  if( debug )
    kDebug() << "==== .Evaluating ..:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();
  session->setContents(unit);

  Control control;
  DumpChain dumper;
  
  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (debug) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, session);
  }

  static int testNumber = 0; //@todo what this url for?
  KUrl url(QString("file:///internal/%1").arg(testNumber++));

  QList<KDevelop::DUContext*> includes;
  includes << context;
  
  DeclarationBuilder definitionBuilder(session);
  TopDUContext* top = definitionBuilder.buildDeclarations(url, ast, &includes);

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);

  if (debug) {
    kDebug() << "===== DUChain:" << endl;

    DUChainWriteLocker lock(DUChain::lock());
    //dumper.dump(top);
  }

  if (debug) {
    kDebug() << "===== Types:" << endl;
    DumpTypes dt;
    foreach (const AbstractType::Ptr& type, definitionBuilder.topTypes())
      dt.dump(type.data());
  }

  if (debug)
    kDebug() << "===== Finished evaluation." << endl;

  delete session;

  return evaluateType( ast, debug );
};

ExpressionEvaluationResult::Pointer ExpressionParser::evaluateType( AST* ast, bool debug ) {
  ExpressionEvaluationResult::Pointer ret( new ExpressionEvaluationResult );
  ret->ast = ast;
  return ret;
}

};