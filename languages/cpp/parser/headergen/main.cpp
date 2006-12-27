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

struct HeaderGeneratorVisitor : public DefaultVisitor
{
  HeaderGeneratorVisitor(ParseSession* session) : inTemplate(false), m_session(session), nc(session) {};

  bool inTemplate;

  ParseSession* m_session;
  NameCompiler nc;

  QList<QString> m_classes;

  QualifiedIdentifier m_currentNS;

  virtual void visitNamespace(NamespaceAST* node)
  {
    QualifiedIdentifier identifier = m_currentNS;

    if (node->namespace_name) {
      QualifiedIdentifier nsID(m_session->token_stream->token(node->namespace_name).symbol());
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
};

class HeaderGenerator : public rpp::Preprocessor
{
public:
  HeaderGenerator();

  void run();

  QString preprocess(const KUrl& url, int sourceLine = -1);

  virtual rpp::Stream* sourceNeeded(QString& fileName, IncludeType type, int sourceLine);

  int status;

private:
  void addWinDeclMacro();

  KUrl kdeIncludes, outputDirectory, folderUrl;

  // path, filename
  QMultiMap<QString, QString> filesToInstall;

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
  HeaderStream(QString* string)
    : Stream(string)
    , m_string(string)
  {
  }

  virtual ~HeaderStream()
  {
    delete m_string;
  }

private:
  QString* m_string;
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
    kDebug() << QString(indent * 2, QChar(' ')) << "Block for file [" << fb->file << "]" << endl;
  else
    kDebug() << QString(indent * 2, QChar(' ')) << "Block, condition" << (elseBlock ? " else" : "") << " [" << block->condition << "]" << endl;

  foreach (rpp::pp_macro* macro, block->macros)
    kDebug() << QString((indent + 1) * 2, QChar(' ')) << "Macro " << macro->name << ", defined " << macro->defined << endl;

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
    if (macro->name == "KDE_EXPORT" || macro->name == "KJS_EXPORT") {
      // Exploit that the parser understands the windows declaration spec stuff
      macro->definition = "__declspec(dllexport)";
    }

    rpp::Environment::setMacro(macro);
  }
};

static KCmdLineOptions options[] =
{
  { "includes <includes>", I18N_NOOP( "KDE include directory - headers go into <includes>/KDE" ), 0 },
  { "buildinfo <buildinfo>", I18N_NOOP( "Build information from the cmake XML generator" ), 0 },
  KCmdLineLastOption // End of options.
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "headergen", I18N_NOOP( "KDE forwarding header generator" ),
                        "0.1", I18N_NOOP( "An application which creates forwarding headers (like Qt headers) from source code." ), KAboutData::License_GPL,
                        I18N_NOOP( "(c) 2006, Hamish Rodda" ), "", "http://www.kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
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
  QDir includeDir(kdeIncludes.path());

  if (!includeDir.exists()) {
    kWarning() << "KDE includes directory must be set, and point to an existing directory; please use --includes <path>" << endl;
    status = -1;
    return;
  }

  if (!includeDir.exists("KDE"))
    if (!includeDir.mkdir("KDE")) {
      kWarning() << "KDE includes directory must be writable" << endl;
      status = -1;
      return;
    }

  outputDirectory = kdeIncludes;
  outputDirectory.cd("KDE");

  outputDir = outputDirectory.path();

  QFile buildInfoFile(args->getOption("buildinfo"));
  if (!buildInfoFile.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open build information file \"" << args->getOption("buildinfo") << "\", please check that it exists and is readable." << endl;
    status = -1;
    return;
  }

  QString errorMsg; int errorLine, errorColumn;
  if (!buildInfoXml.setContent(buildInfoFile.readAll(), false, &errorMsg, &errorLine, &errorColumn)) {
    kWarning() << "Build information parse error \"" << errorMsg << "\" at line " << errorLine << ", column " << errorColumn << endl;
    status = -1;
    return;
  }

  preprocessor.setEnvironment(new OverridingEnvironment(&preprocessor));

  topBlock = new FileBlock;
  topBlock->file = "<internal pp>";

  rpp::pp_macro* exportMacro = new rpp::pp_macro;
  exportMacro->name = "__cplusplus";
  exportMacro->definition = "1";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "__GNUC__";
  exportMacro->definition = "4";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "__GNUC_MINOR__";
  exportMacro->definition = "1";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "__linux__";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "KDE_EXPORT";
  exportMacro->definition = "__declspec(dllexport)";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "KJS_EXPORT";
  exportMacro->definition = "__declspec(dllexport)";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);

  exportMacro = new rpp::pp_macro;
  exportMacro->name = "Q_WS_X11";
  exportMacro->function_like = false;
  exportMacro->variadics = false;
  topBlock->setMacro(exportMacro);
}

QString HeaderGenerator::preprocess(const KUrl& url, int sourceLine)
{
  //kDebug() << k_funcinfo << url << endl;

  preprocessing.push(url);

  QFile sourceToParse(url.path());
  if (!sourceToParse.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open install file " << url << endl;
    return QByteArray();
  }

  FileBlock* fileMacros = new FileBlock;
  fileMacros->file = url.path();
  headerMacros.insert(url.path(), fileMacros);

  preprocessor.environment()->enterBlock(fileMacros);

  QString ret = preprocessor.processFile(QString(sourceToParse.readAll()), rpp::pp::Data);

  preprocessor.environment()->leaveBlock();

  preprocessing.pop();

  return ret;
}

rpp::Stream* HeaderGenerator::sourceNeeded(QString& fileName, IncludeType /*type*/, int sourceLine)
{
  //kDebug() << k_funcinfo << fileName << " from " << preprocessing.top() << endl;

  if (fileName.endsWith("kkeydialog.h"))
    kDebug() << "Maybe parsing " << fileName << endl;

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
      if (QFile::exists(url.path())) {
        // found it
        if (headerMacros.contains(url.path())) {
          FileBlock* macros = headerMacros[url.path()];
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

  kWarning() << "Did not find include " << fileName <<endl;//<< " in urls " << toTry << endl;//" in the following directories:" << endl;
  /*QDomNodeList include = folderElement.elementsByTagName("include");
  for (int i = 0; i < include.count(); ++i)
    kDebug() << "  " << include.at(i).toElement().text() << endl;*/

  return 0;
}

void printMacros(rpp::Environment* environment)
{
  kDebug() << "Macros for environment:" << endl;
  foreach (rpp::pp_macro* macro, environment->allMacros())
    kDebug() << "  Macro [" << macro->name << "] " << (macro->defined ? " [" : "undefined") << macro->definition << (macro->defined ? "]" : "") << endl;
}

void HeaderGenerator::run()
{
  QDomNodeList folders = buildInfoXml.elementsByTagName("folder");
  QString sourceElementName("source"), installElementName("install");

  int fileCount = 0;

  QFile kdelibsExport(kdeIncludes.path(KUrl::AddTrailingSlash) + "kdelibs_export.h");
  if (!kdelibsExport.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open kdelibs_export.h in kde includes directory.  Are you sure you have installed kdelibs?" << endl;
    status = -1;
    return;
  }

  for (int i = 0; i < folders.count(); ++i) {
    folderElement = folders.at(i).toElement();
    folderUrl = KUrl(folderElement.attribute("name") + "/");

    for (QDomElement install = folderElement.firstChildElement(installElementName); !install.isNull(); install = install.nextSiblingElement(installElementName)) {
      KUrl installDestination(install.attribute("destination"));
      if (!installDestination.path().startsWith(kdeIncludes.path())) {
        continue;
      }

      for (QDomElement source = install.firstChildElement(sourceElementName); !source.isNull(); source = source.nextSiblingElement(sourceElementName)) {
        if (source.text().endsWith("kkeydialog.h"))
          //continue;
          kDebug() << "Parsing " << source.text() << endl;

        preprocessor.environment()->clear();
        preprocessor.environment()->visitBlock(topBlock);

        QByteArray contents = preprocess(source.text()).toLatin1();

        if (contents.isEmpty()) {
          kWarning() << "Contents empty for " << source.text() << endl;
          continue;
        }

        ++fileCount;

        Control control;

        ParseSession session;
        session.setContents(contents);

        Parser parser(&control);
        TranslationUnitAST* ast = parser.parse(&session);

        HeaderGeneratorVisitor hg(&session);
        hg.visit(ast);

        if (hg.m_classes.isEmpty()) {
          kWarning() << "Parse problem in " << source.text() << ": no classes found!" << endl;

          //hg.m_classes.append(source.text().mid(source.text().lastIndexOf('/')).replace('.', '_'));

          // << QString::fromUtf8(contents).trimmed() << endl;

          //printMacros(headerMacros[source.text()]);

          //kFatal() << "bye :)" << endl;
        }

        if (source.text().endsWith("kkeydialog.h")) {
          kDebug() << "Parse " << source.text() << ": " << hg.m_classes.count() << " classes found:" << hg.m_classes << " lines " << contents.count('\n') << endl
          << QString::fromUtf8(contents) << endl;

#if 0
          DumpTree dt;
          dt.dump(ast, session.token_stream);

          for (int i = 0; i < control.problemCount(); ++i)
            kWarning() << "Parse problem in " << source.text() << ": " << control.problem(i).message() << ", line " << control.problem(i).line() << endl;
#endif

          visitBlock(headerMacros[source.text()]);
          printMacros(preprocessor.environment());

          //kFatal() << "bye :)" << endl;
        }

        foreach (const QString& className, hg.m_classes) {
          QString forwardingHeaderPath(outputDirectory.path(KUrl::AddTrailingSlash) + className);
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

            kWarning() << "Could not open forwarding header file " << forwardingHeaderPath << endl;
            continue;
          }

          success:

          QTextStream ts(&forwardingHeader);

          QString sourceRelativeInstallPath = installDestination.path().mid(kdeIncludes.path().length() + 1);
          if (!sourceRelativeInstallPath.isEmpty())
            sourceRelativeInstallPath += '/';

          KUrl sourceUrl(source.text());
          QString sourceRelativeUrl = sourceRelativeInstallPath + source.text().mid(folderUrl.path().length());

          ts << "#include \"";

          int dotdotcount = className.count('/');
          for (int i = 0; i <= dotdotcount; ++i)
            ts << QString("../");

          ts << sourceRelativeInstallPath << sourceUrl.fileName() << "\"\n";

          //ts << QString::fromUtf8(contents) << "\n";

          filesToInstall.insert(classDirectory, className);
        }
      }
    }
  }

  QFile cmakelist(outputDirectory.path(KUrl::AddTrailingSlash) + "CMakeLists.txt");
  if (!cmakelist.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    kWarning() << "Could not open cmake list file for writing." << endl;
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
      ts << "install( FILES " << endl;

      QStringList classes = filesToInstall.values(directory);
      classes.sort();

      foreach (const QString& className, classes)
        ts << "  " << className << endl;

      ts << "DESTINATION ${INCLUDE_INSTALL_DIR}/KDE" << (!directory.isEmpty() ? "/" : "") << directory << " )" << endl;
      ts << endl;
    }
  }

  kDebug() << "Finished parsing " << fileCount << " files." << endl;
}
