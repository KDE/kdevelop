#include <QByteArray>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QTextStream>
#include <QMultiMap>
#include <QMapIterator>

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kurl.h>

#include "parser.h"
#include "parsesession.h"
#include "control.h"
#include "name_compiler.h"
#include "rpp/pp-stream.h"

struct HeaderGeneratorVisitor : public DefaultVisitor
{
  HeaderGeneratorVisitor(ParseSession* session) : m_session(session), nc(session) {};

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

  virtual void visitClassSpecifier(ClassSpecifierAST* node)
  {
    if (!node->win_decl_specifiers)
      return;

    QualifiedIdentifier identifier = m_currentNS;

    nc.run(node->name);
    QualifiedIdentifier classIdentifier = nc.identifier().merge(m_currentNS);

    m_classes.append(classIdentifier.toString().replace("::", "/"));

    m_currentNS = classIdentifier.merge(m_currentNS);

    DefaultVisitor::visitClassSpecifier(node);

    m_currentNS = identifier;
  }
};

class HeaderGenerator : public Preprocessor
{
public:
  HeaderGenerator();

  void run();

  QByteArray preprocess(const KUrl& url);

  virtual Stream* sourceNeeded(QString& fileName, IncludeType type);

  int status;

private:
  void addWinDeclMacro();

  KUrl kdeIncludes, outputDirectory;

  // path, filename
  QMultiMap<QString, QString> filesToInstall;

  QDir outputDir;

  QDomDocument buildInfoXml;
  QDomElement folderElement;

  // Map of macros in each include file
  QMap<QString, QList<MacroItem> > headerMacros;

  //QStack includeUrls;
};

class HeaderStream : public Stream
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
}

QByteArray HeaderGenerator::preprocess(const KUrl& url)
{
  //if (url.fileName() == "qglobal.h")
//    kDebug() << "Parsing qglobal.h... " << url.path() << endl;

  QFile sourceToParse(url.path());
  if (!sourceToParse.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open install file " << url << endl;
    return QByteArray();
  }

  /*if (url.fileName() == "kdelibs_export.h")
    kDebug() << "KDE libs export macros encountered." << endl;*/

  QByteArray ret = processString(QString(sourceToParse.readAll())).toLatin1();

  if (url.fileName() == "cursor.h") {
    kDebug() << "Defined macros:" << endl;
    foreach (MacroItem item, macros())
      kDebug() << "Macro [" << item.name << "] definition [" << item.definition << "]" << endl;
  }

  //kDebug() << "Inserting " << macros().count() << " macros into record for " << url.path() << endl;
  headerMacros.insert(url.path(), macros());

  return ret;
}

Stream* HeaderGenerator::sourceNeeded(QString& fileName, IncludeType /*type*/)
{
  QDomNodeList includes = folderElement.elementsByTagName("includes");

  for (int i2 = 0; i2 < includes.count(); ++i2) {
    QDomNodeList include = includes.at(i2).toElement().elementsByTagName("include");

    for (int i = 0; i < include.count(); ++i) {
      KUrl path(include.at(i).toElement().text() + '/');
      KUrl url(path, fileName);

      //kDebug() << "Checking file " << fileName << " in url " << url.path() << " from path " << path.path() << endl;

      if (url.isValid()) {
        if (QFile::exists(url.path())) {
          /*QFile file(url.path());
          if (!file.open(QIODevice::ReadOnly)) {
            kWarning() << "Could not open file " << url.path() << " for reading!" << endl;
            return 0;
          }

          addWinDeclMacro();

          QString* input = new QString(QString::fromUtf8(file.readAll()));
          return new HeaderStream(input);*/
          // found it
          if (headerMacros.contains(url.path())) {
            QList<MacroItem> ms = headerMacros[url.path()];
            //kDebug() << "Retrieved " << ms.count() << " macros from record of " << url.path() << endl;
            int oldMacroCount = macros().count();
            addMacros(ms);
            //kDebug() << "New macro count " << macros().count() << ", old " << oldMacroCount << endl;
            addWinDeclMacro();
            return 0;

          } else {
            // The caching isn't going to be 100% correct, but it will be good enough for what we need
            preprocess(url);
            addWinDeclMacro();
            return 0;
          }
        }
      }

      //kDebug() << "File " << fileName << " not in url " << url.path() << endl;
    }
  }

  //kWarning() << "Did not find include " << fileName << endl;//" in the following directories:" << endl;
  /*for (int j = 0; j < include.count(); ++j)
    kDebug() << "  " << include.at(j).toElement().text() << endl;*/

  return 0;
}

void HeaderGenerator::addWinDeclMacro()
{
  // Use the fact that the c++ parser recognises windows declaration specs
  MacroItem exportMacro;
  exportMacro.name = "KDE_EXPORT";
  exportMacro.isDefined = true;
  exportMacro.definition = "__declspec(dllexport)";
  exportMacro.isFunctionLike = false;
  exportMacro.variadics = false;
  addMacros(QList<MacroItem>() << exportMacro);
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
    KUrl folderUrl(folderElement.attribute("name"));

    for (QDomElement install = folderElement.firstChildElement(installElementName); !install.isNull(); install = install.nextSiblingElement(installElementName)) {
      KUrl installDestination(install.attribute("destination"));
      if (!installDestination.path().startsWith(kdeIncludes.path())) {
        //kDebug() << "URL " << installDestination << " is not the kde include dir " << kdeIncludes << endl;
        continue;
      }

      for (QDomElement source = install.firstChildElement(sourceElementName); !source.isNull(); source = source.nextSiblingElement(sourceElementName)) {
        clearMacros();

        // Use the fact that the c++ parser recognises windows declaration specs
        addWinDeclMacro();

        MacroItem cppmacro;
        cppmacro.name = "__cplusplus";
        cppmacro.isDefined = true;
        cppmacro.isFunctionLike = false;
        cppmacro.variadics = false;

        addMacros(QList<MacroItem>() << cppmacro);

        if (source.text().endsWith("cursor.h"))
          kDebug() << "Cursor encountered" << endl;

        QByteArray contents = preprocess(source.text());

        if (contents.isEmpty())
          continue;

        if (source.text().endsWith("cursor.h"))
          kDebug() << QString::fromUtf8(contents).trimmed() << endl;

        ++fileCount;

        Control control;

        ParseSession session;
        session.setContents(contents);

        Parser parser(&control);
        TranslationUnitAST* ast = parser.parse(&session);

        //for (int i = 0; i < control.problemCount(); ++i)
          //kWarning() << "Parse problem in " << source.text() << ": " << control.problem(i).message() << ", line " << control.problem(i).line() << endl;

        HeaderGeneratorVisitor hg(&session);
        hg.visit(ast);



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

          QString sourceRelativeUrl = sourceRelativeInstallPath + source.text().mid(folderUrl.path().length() + 1);

          ts << "#include \"";

          int dotdotcount = className.count('/');
          for (int i = 0; i <= dotdotcount; ++i)
            ts << QString("../");

          ts << sourceRelativeUrl << "\"\n";

          /*ts << contents << endl;

          ts << "Defined macros:" << endl;
          foreach (MacroItem item, macros())
            ts << "Macro [" << item.name << "] definition [" << item.definition << "] from [" << item.fileName << "]" << endl;*/

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
  QString directory = "/";

  QMapIterator<QString, QString> it = filesToInstall;
  while (it.hasNext()) {
    it.next();
    if (it.key() != directory) {
      if (directory != "/") {
        if (!directory.isEmpty())
          directory.prepend('/');
        ts << "DESTINATION ${INCLUDE_INSTALL_DIR}/KDE" << directory << " )" << endl;
        ts << endl;
      }
      ts << endl;
      ts << "install( FILES " << endl;

      directory = it.key();
    }

    ts << "  " << it.value() << endl;
  }

  if (directory != "/") {
    if (!directory.isEmpty())
      directory.prepend('/');
    ts << "DESTINATION ${INCLUDE_INSTALL_DIR}/KDE" << directory << " )" << endl;
    ts << endl;
  }

  kDebug() << "Finished parsing " << fileCount << " files." << endl;
}
