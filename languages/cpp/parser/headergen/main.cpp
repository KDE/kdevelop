/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QByteArray>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QTextStream>
#include <QMultiMap>
#include <QMapIterator>
#include <QSet>

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kurl.h>

#include "parser.h"
#include "parsesession.h"
#include "control.h"
#include "name_compiler.h"
#include "dumptree.h"

#include "rpp/pp-stream.h"
#include "rpp/pp-environment.h"
#include "rpp/preprocessor.h"
#include "rpp/pp-macro.h"
#include "rpp/pp-engine.h"
#include "rpp/chartools.h"
#include "rpp/macrorepository.h"

using namespace KDevelop;

struct HeaderGeneratorVisitor : public DefaultVisitor
{
  HeaderGeneratorVisitor(ParseSession* session) : inTemplate(false), m_session(session), nc(session) {}

  bool inTemplate;

  ParseSession* m_session;
  NameCompiler nc;

  QList<QString> m_classes;
  QSet<QString> m_namespaces;

  QualifiedIdentifier m_currentNS;

  virtual void visitNamespace(NamespaceAST* node)
  {
    QualifiedIdentifier identifier = m_currentNS;

    if (node->namespace_name) {
      QualifiedIdentifier nsID(m_session->token_stream->token(node->namespace_name).symbolString());
      m_currentNS = nsID.merge(m_currentNS);
    }

    DefaultVisitor::visitNamespace(node);

    m_currentNS = identifier;
  }

  virtual void visitTemplateDeclaration(TemplateDeclarationAST* node)
  {
    bool wasInTemplate = inTemplate;
    inTemplate = true;

    DefaultVisitor::visitTemplateDeclaration(node);

    inTemplate = wasInTemplate;
  }

  virtual void visitClassSpecifier(ClassSpecifierAST* node)
  {
    if (!node->win_decl_specifiers && !inTemplate)
      return;

    QualifiedIdentifier identifier = m_currentNS;

    nc.run(node->name);
    QualifiedIdentifier classIdentifier = nc.identifier().merge(m_currentNS);

    m_classes.append(classIdentifier.toString().replace("::", "/"));

    m_currentNS = classIdentifier.merge(m_currentNS);

    // Class-in-class doesn't work (can't have a file and a directory with the same name)
    // so don't bother
    //DefaultVisitor::visitClassSpecifier(node);

    m_currentNS = identifier;
  }

  virtual void visitFunctionDefinition(FunctionDefinitionAST* node)
  {
    if (!node->win_decl_specifiers)
      return;

    if (m_currentNS.isEmpty())
      return;

    m_namespaces.insert(m_currentNS.toString().replace("::", "/"));
  }
};

class HeaderGenerator : public rpp::Preprocessor
{
public:
  HeaderGenerator();

  void run();

  QString preprocess(const KUrl& url, int sourceLine = -1);

  virtual rpp::Stream* sourceNeeded(QString& fileName, IncludeType type, int sourceLine, bool skipCurrentPath);

  int status;

private:
  void addWinDeclMacro();

  KUrl kdeIncludes, outputDirectory, folderUrl;

  // path, filename
  QMultiMap<QString, QString> filesToInstall;

  // namespace, filename
  QMultiMap<QString, QString> namespaceDeclarations;

  QDir outputDir;

  QDomDocument buildInfoXml;
  QDomElement folderElement;

  // Map of macros in each include file
  QMap<QString, class FileBlock*> headerMacros;

  FileBlock* topBlock;

  QStack<FileBlock*> currentFileBlocks;
  QStack<KUrl> preprocessing;

  rpp::pp preprocessor;
  //QStack includeUrls;
};

class HeaderStream : public rpp::Stream
{
public:
  HeaderStream(QByteArray* string)
    : Stream(new PreprocessedContents(tokenizeFromByteArray(*string)))
    , m_string(string)
  {
  }

  virtual ~HeaderStream()
  {
    delete m_string;
  }

private:
  QByteArray* m_string;
};

class FileBlock : public rpp::MacroBlock
{
public:
  FileBlock() : rpp::MacroBlock(0) {}

  QString file;
};


void visitBlock(rpp::MacroBlock* block, int indent = 0, bool elseBlock = false)
{
  static QSet<rpp::MacroBlock*> encountered;

  if (encountered.contains(block))
    return;

  encountered.insert(block);

  ++indent;
  if (FileBlock* fb = dynamic_cast<FileBlock*>(block))
    kDebug(9007) << QString(indent * 2, QChar(' ')) << "Block for file [" << fb->file << "]";
  else
    kDebug(9007) << QString(indent * 2, QChar(' ')) << "Block, condition" << (elseBlock ? " else" : "") << "[" << block->condition << "]";

  foreach (rpp::pp_macro* macro, block->macros)
    kDebug(9007) << QString((indent + 1) * 2, QChar(' ')) << "Macro" << macro->name.str() << ", defined" << macro->defined;

  foreach (rpp::MacroBlock* child, block->childBlocks)
    visitBlock(child, indent);

  --indent;

  if (block->elseBlock)
    visitBlock(block->elseBlock, indent, true);

  if (indent == 0)
    encountered.clear();
}

class OverridingEnvironment : public rpp::Environment
{
public:
  OverridingEnvironment(rpp::pp* preprocessor) : rpp::Environment(preprocessor) {}

  virtual void setMacro(rpp::pp_macro* macro)
  {
    rpp::Environment::setMacro(macro);
  }
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "headergen", 0, ki18n( "KDE forwarding header generator" ),
                        "0.1", ki18n( "An application which creates forwarding headers (like Qt headers) from source code." ), KAboutData::License_GPL,
                        ki18n( "(c) 2006, Hamish Rodda" ), KLocalizedString(), "http://www.kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("includes <includes>", kxi18n( "KDE include directory - headers go into <placeholder>includes</placeholder>/KDE" ));
  options.add("buildinfo <buildinfo>", ki18n( "Build information from the cmake XML generator" ));
  KCmdLineArgs::addCmdLineOptions( options );

  HeaderGenerator hg;

  if (hg.status != 0)
    return hg.status;

  hg.run();

  return hg.status;
}

HeaderGenerator::HeaderGenerator()
  : status(0)
  , preprocessor(this)
{
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  kdeIncludes = args->getOption("includes");
  QDir includeDir(kdeIncludes.toLocalFile());

  if (!includeDir.exists()) {
    kWarning() << "KDE includes directory must be set, and point to an existing directory; please use --includes <path>" ;
    status = -1;
    return;
  }

  if (!includeDir.exists("KDE"))
    if (!includeDir.mkdir("KDE")) {
      kWarning() << "KDE includes directory must be writable" ;
      status = -1;
      return;
    }

  outputDirectory = kdeIncludes;
  outputDirectory.cd("KDE");

  outputDir = outputDirectory.toLocalFile();

  QFile buildInfoFile(args->getOption("buildinfo"));
  if (!buildInfoFile.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open build information file \"" << args->getOption("buildinfo") << "\", please check that it exists and is readable." ;
    status = -1;
    return;
  }

  QString errorMsg; int errorLine, errorColumn;
  if (!buildInfoXml.setContent(buildInfoFile.readAll(), false, &errorMsg, &errorLine, &errorColumn)) {
    kWarning() << "Build information parse error \"" << errorMsg << "\" at line" << errorLine << ", column" << errorColumn ;
    status = -1;
    return;
  }

  preprocessor.setEnvironment(new OverridingEnvironment(&preprocessor));

  topBlock = new FileBlock;
  topBlock->file = "<internal pp>";

  rpp::pp_macro* exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("__cplusplus");
  exportMacro->definition = tokenizeFromByteArray("1");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("__GNUC__");
  exportMacro->definition = tokenizeFromByteArray("4");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("__GNUC_MINOR__");
  exportMacro->definition = tokenizeFromByteArray("1");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("__linux__");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("KDE_EXPORT");
  exportMacro->definition = tokenizeFromByteArray("__declspec(dllexport)");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("KJS_EXPORT");
  exportMacro->definition = tokenizeFromByteArray("__declspec(dllexport)");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));

  exportMacro = new rpp::pp_macro;
  exportMacro->name = IndexedString("Q_WS_X11");
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(makeConstant(exportMacro));
}

QString HeaderGenerator::preprocess(const KUrl& url, int sourceLine)
{
  //kDebug(9007) << url;

  preprocessing.push(url);

  QFile sourceToParse(url.toLocalFile());
  if (!sourceToParse.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open install file" << url ;
    return QByteArray();
  }

  FileBlock* fileMacros = new FileBlock;
  fileMacros->file = url.toLocalFile();
  headerMacros.insert(url.toLocalFile(), fileMacros);

  preprocessor.environment()->enterBlock(fileMacros);

  QByteArray ret = stringFromContents(preprocessor.processFile(url.pathOrUrl(), sourceToParse.readAll()));

  preprocessor.environment()->leaveBlock();

  preprocessing.pop();

  return ret;
}

rpp::Stream* HeaderGenerator::sourceNeeded(QString& fileName, IncludeType /*type*/, int sourceLine, bool skipCurrentPath)
{
  //kDebug(9007) << fileName << "from" << preprocessing.top();

  KUrl::List toTry;

  {
    KUrl url(folderUrl, fileName);

    toTry << url;
  }

  for (QDomElement includes = folderElement.firstChildElement("includes"); !includes.isNull(); includes = includes.nextSiblingElement("includes")) {
    for (QDomElement include = includes.firstChildElement("include"); !include.isNull(); include = include.nextSiblingElement("include")) {
      KUrl path(include.toElement().text() + '/');
      KUrl url(path, fileName);

      toTry << url;
    }
  }

  toTry << KUrl("/usr/include/linux/" + fileName);

  foreach (const KUrl& url, toTry) {
    if (url.isValid()) {
      if (QFile::exists(url.toLocalFile())) {
        // found it
        if (headerMacros.contains(url.toLocalFile())) {
          FileBlock* macros = headerMacros[url.toLocalFile()];
          preprocessor.environment()->visitBlock(macros);
          preprocessor.environment()->currentBlock()->childBlocks.append(macros);
          return 0;

        } else {
          // The caching isn't going to be 100% correct, but it will be good enough for what we need
          preprocess(url, sourceLine);
          return 0;
        }
      }
    }
  }

  kWarning() << "Did not find include" << fileName;//<< "in urls" << toTry << endl;//" in the following directories:" ;
  /*QDomNodeList include = folderElement.elementsByTagName("include");
  for (int i = 0; i < include.count(); ++i)
    kDebug(9007) << "" << include.at(i).toElement().text();*/

  return 0;
}

void printMacros(rpp::Environment* environment)
{
  kDebug(9007) << "Macros for environment:";
  foreach (rpp::pp_macro* macro, environment->allMacros())
    kDebug(9007) << "Macro [" << macro->name.str() << "]" << (macro->defined ? " [" : "undefined") << QString::fromUtf8(stringFromContents(macro->definition(), macro->definitionSize())) << (macro->defined ? "]" : "");
}

void HeaderGenerator::run()
{
  QDomNodeList folders = buildInfoXml.elementsByTagName("folder");
  QString sourceElementName("source"), installElementName("install");

  int fileCount = 0;

  QFile kdelibsExport(kdeIncludes.toLocalFile(KUrl::AddTrailingSlash) + "kdelibs_export.h");
  if (!kdelibsExport.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open kdelibs_export.h in kde includes directory.  Are you sure you have installed kdelibs?" ;
    status = -1;
    return;
  }

  for (int i = 0; i < folders.count(); ++i) {
    folderElement = folders.at(i).toElement();
    folderUrl = KUrl(folderElement.attribute("name") + '/');

    for (QDomElement install = folderElement.firstChildElement(installElementName); !install.isNull(); install = install.nextSiblingElement(installElementName)) {
      KUrl installDestination(install.attribute("destination"));
      if (!installDestination.toLocalFile().startsWith(kdeIncludes.toLocalFile())) {
        continue;
      }

      for (QDomElement source = install.firstChildElement(sourceElementName); !source.isNull(); source = source.nextSiblingElement(sourceElementName)) {
        //if (source.text().endsWith("kkeydialog.h"))
          //continue;
          //kDebug(9007) << "Parsing" << source.text();

        preprocessor.environment()->clear();
        preprocessor.environment()->visitBlock(topBlock);

        QByteArray contents = preprocess(source.text()).toLatin1();

        if (contents.isEmpty()) {
          kWarning() << "Contents empty for" << source.text() ;
          continue;
        }

        ++fileCount;

        Control control;

        ParseSession session;
        session.setContents(tokenizeFromByteArray(contents), preprocessor.environment()->takeLocationTable());

        Parser parser(&control);
        TranslationUnitAST* ast = parser.parse(&session);

        HeaderGeneratorVisitor hg(&session);
        hg.visit(ast);

        if (hg.m_classes.isEmpty()) {
          kWarning() << "Parse problem in" << source.text() << ": no classes found!" ;

          //hg.m_classes.append(source.text().mid(source.text().lastIndexOf('/')).replace('.', '_'));

          // << QString::fromUtf8(contents).trimmed() << endl;

          //printMacros(headerMacros[source.text()]);

          //kFatal() << "bye :)";
        }

        if (false && source.text().endsWith("kkeydialog.h")) {
          kDebug(9007) << "Parse" << source.text() << ":" << hg.m_classes.count() << "classes found:" << hg.m_classes << "lines" << contents.count('\n') << endl
          << QString::fromUtf8(contents) << endl;

#if 0
          DumpTree dt;
          dt.dump(ast, session.token_stream);

          for (int i = 0; i < control.problemCount(); ++i)
            kWarning() << "Parse problem in" << source.text() << ":" << control.problem(i).message() << ", line" << control.problem(i).line() ;
#endif

          visitBlock(headerMacros[source.text()]);
          printMacros(preprocessor.environment());

          //kFatal() << "bye :)";
        }

        foreach (const QString& className, hg.m_classes) {
          QString forwardingHeaderPath(outputDirectory.toLocalFile(KUrl::AddTrailingSlash) + className);
          QString classDirectory;
          int index = className.lastIndexOf('/');
          if (index > 0)
            classDirectory = className.left(index);

          QFile forwardingHeader(forwardingHeaderPath);
          if (!forwardingHeader.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            if (!classDirectory.isEmpty() && outputDir.mkpath(classDirectory)) {
              if (forwardingHeader.open(QIODevice::WriteOnly | QIODevice::Truncate))
                goto success;
            }

            kWarning() << "Could not open forwarding header file" << forwardingHeaderPath ;
            continue;
          }

          success:

          QTextStream ts(&forwardingHeader);

          QString sourceRelativeInstallPath = installDestination.toLocalFile().mid(kdeIncludes.toLocalFile().length() + 1);
          if (!sourceRelativeInstallPath.isEmpty())
            sourceRelativeInstallPath += '/';

          KUrl sourceUrl(source.text());
          QString sourceRelativeUrl = sourceRelativeInstallPath + source.text().mid(folderUrl.toLocalFile().length());

          QString dotdot;
          int dotdotcount = className.count('/');
          for (int i = 0; i <= dotdotcount; ++i)
            dotdot.append("../");

          QString includeFile = QString("#include \"%1%2%3\"\n").arg(dotdot).arg(sourceRelativeInstallPath).arg(sourceUrl.fileName());

          ts << includeFile;

          filesToInstall.insert(classDirectory, className);

          foreach (const QString& ns, hg.m_namespaces)
            namespaceDeclarations.insert(ns, includeFile);
        }
      }
    }
  }

  {
    QTextStream *ts = 0;
    QMapIterator<QString, QString> it = namespaceDeclarations;
    QString currentNS;

    while (it.hasNext()) {
      it.next();
      if (currentNS != it.key()) {
        currentNS = it.key();
        delete ts;
        ts = 0;

        if (filesToInstall.contains(it.key()))
          continue;

        QString namespaceName = it.key();

        // Found a namespace without a class... install a file for it
        QString forwardingHeaderPath(outputDirectory.toLocalFile(KUrl::AddTrailingSlash) + namespaceName);
        QString namespaceDirectory;
        int index = namespaceName.lastIndexOf('/');
        if (index > 0)
          namespaceDirectory = namespaceName.left(index);

        QFile forwardingHeader(forwardingHeaderPath);
        if (!forwardingHeader.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
          if (!namespaceDirectory.isEmpty() && outputDir.mkpath(namespaceDirectory)) {
            if (forwardingHeader.open(QIODevice::WriteOnly | QIODevice::Truncate))
              goto success2;
          }

          kWarning() << "Could not open forwarding header file" << forwardingHeaderPath ;
          continue;
        }

        success2:

        ts = new QTextStream(&forwardingHeader);

        filesToInstall.insert(namespaceDirectory, namespaceName);
      }

      if (ts)
        *ts << it.value();
    }

    delete ts;
    ts = 0;
  }

  QFile cmakelist(outputDirectory.toLocalFile(KUrl::AddTrailingSlash) + "CMakeLists.txt");
  if (!cmakelist.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    kWarning() << "Could not open cmake list file for writing." ;
    status = -1;
    return;
  }

  QTextStream ts(&cmakelist);

  // Phony starting directory
  QString directory = QDir::rootPath();

  QMapIterator<QString, QString> it = filesToInstall;
  while (it.hasNext()) {
    it.next();

    if (it.key() != directory) {
      directory = it.key();

      ts << endl;
      ts << "install( FILES" << endl;

      QSet<QString> classes2 = QSet<QString>::fromList(filesToInstall.values(directory));
      QStringList classes = classes2.toList();
      classes.sort();

      foreach (const QString& className, classes)
        ts << "" << className << endl;

      ts << "DESTINATION ${INCLUDE_INSTALL_DIR}/KDE" << (!directory.isEmpty() ? "/" : "") << directory << ")" << endl;
      ts << endl;
    }
  }

  kDebug(9007) << "Finished parsing" << fileCount << "files.";
}
