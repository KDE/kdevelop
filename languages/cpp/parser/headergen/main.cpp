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
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  KUrl kdeIncludes(args->getOption("includes"));
  QDir includeDir(kdeIncludes.path());

  if (!includeDir.exists()) {
    kWarning() << "KDE includes directory must be set, and point to an existing directory; please use --includes <path>" << endl;
    return -1;
  }

  if (!includeDir.exists("KDE"))
    if (!includeDir.mkdir("KDE")) {
      kWarning() << "KDE includes directory must be writable" << endl;
      return -1;
    }

  KUrl outputDirectory = kdeIncludes;
  outputDirectory.cd("KDE");

  QDir outputDir(outputDirectory.path());

  QFile buildInfoFile(args->getOption("buildinfo"));
  if (!buildInfoFile.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open build information file \"" << args->getOption("buildinfo") << "\", please check that it exists and is readable." << endl;
    return -1;
  }

  QDomDocument buildInfoXml;
  QString errorMsg; int errorLine, errorColumn;
  if (!buildInfoXml.setContent(buildInfoFile.readAll(), false, &errorMsg, &errorLine, &errorColumn)) {
    kWarning() << "Build information parse error \"" << errorMsg << "\" at line " << errorLine << ", column " << errorColumn << endl;
    return -1;
  }

  Control control;

  QDomNodeList folders = buildInfoXml.elementsByTagName("folder");
  QString sourceElementName("source"), installElementName("install");

  int fileCount = 0;

  QFile kdelibsExport(kdeIncludes.path(KUrl::AddTrailingSlash) + "kdelibs_export.h");
  if (!kdelibsExport.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open kdelibs_export.h in kde includes directory.  Are you sure you have installed kdelibs?" << endl;
    return -1;
  }

  // Exploit that the c++ parser treats this specially for us :)
  QByteArray exportMacros = "#define KDE_EXPORT __declspec(dllexport)\n" + kdelibsExport.readAll();

  // path, filename
  QMultiMap<QString, QString> filesToInstall;

  for (int i = 0; i < folders.count(); ++i) {
    KUrl folderUrl(folders.at(i).toElement().attribute("name"));

    for (QDomElement install = folders.at(i).firstChildElement(installElementName); !install.isNull(); install = install.nextSiblingElement(installElementName)) {
      KUrl installDestination(install.attribute("destination"));
      if (!installDestination.path().startsWith(kdeIncludes.path())) {
        //kDebug() << "URL " << installDestination << " is not the kde include dir " << kdeIncludes << endl;
        continue;
      }

      for (QDomElement source = install.firstChildElement(sourceElementName); !source.isNull(); source = source.nextSiblingElement(sourceElementName)) {
        QFile sourceToParse(source.text());
        if (!sourceToParse.open(QIODevice::ReadOnly)) {
          kWarning() << "Could not open install file " << source.text() << endl;
          continue;
        }

        ++fileCount;

        QByteArray contents = exportMacros + sourceToParse.readAll();

        Preprocessor p;

        contents = p.processString(QString(contents)).toLatin1();

        ParseSession session;
        session.setContents(contents);

        Parser parser(&control);
        TranslationUnitAST* ast = parser.parse(&session);

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

          filesToInstall.insert(classDirectory, className);
        }
      }
    }
  }

  QFile cmakelist(outputDirectory.path(KUrl::AddTrailingSlash) + "CMakeLists.txt");
  if (!cmakelist.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    kWarning() << "Could not open cmake list file for writing." << endl;
    return -1;
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
