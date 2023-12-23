/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeutils.h"
#include "cmakeprojectdata.h"

#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QRegularExpression>

#include <KConfigGroup>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iplugincontroller.h>
#include <QStandardPaths>

#include "icmakedocumentation.h"
#include "cmakebuilddirchooser.h"
#include "cmakeconfiggroupkeys.h"
#include "settings/cmakecachemodel.h"
#include "debug.h"
#include "cmakebuilderconfig.h"
#include <cmakecachereader.h>
#include "parser/cmakelistsparser.h"

using namespace KDevelop;

namespace
{

KConfigGroup baseGroup( KDevelop::IProject* project )
{
    if (!project)
        return KConfigGroup();

    return project->projectConfiguration()->group( Config::groupName );
}

KConfigGroup buildDirGroup( KDevelop::IProject* project, int buildDirIndex )
{
    return baseGroup(project).group(Config::groupNameBuildDir(buildDirIndex));
}

bool buildDirGroupExists( KDevelop::IProject* project, int buildDirIndex )
{
    return baseGroup(project).hasGroup(Config::groupNameBuildDir(buildDirIndex));
}

QString readBuildDirParameter(KDevelop::IProject* project, const QString &key, const QString& aDefault, int buildDirectory)
{
    const int buildDirIndex = buildDirectory<0 ? CMake::currentBuildDirIndex(project) : buildDirectory;
    if (buildDirIndex >= 0) // NOTE: we return trimmed since we may have written bogus trailing newlines in the past...
        return buildDirGroup( project, buildDirIndex ).readEntry( key, aDefault ).trimmed();
    else
        return aDefault;
}

void writeBuildDirParameter(KDevelop::IProject* project, const QString &key, const QString& value)
{
    int buildDirIndex = CMake::currentBuildDirIndex(project);
    if (buildDirIndex >= 0)
    {
        KConfigGroup buildDirGrp = buildDirGroup( project, buildDirIndex );
        buildDirGrp.writeEntry( key, value );
    }

    else
    {
        qCWarning(CMAKE) << "cannot write key" << key << "(" << value << ")" << "when no builddir is set!";
    }
}

template <typename Key>
void writeProjectBaseParameter(KDevelop::IProject* project, const Key& key, const QString& value)
{
    baseGroup(project).writeEntry(key, value);
}

void setBuildDirRuntime( KDevelop::IProject* project, const QString& name)
{
    writeBuildDirParameter(project, Config::Specific::buildDirRuntime, name);
}

QString buildDirRuntime( KDevelop::IProject* project, int builddir)
{
    return readBuildDirParameter(project, Config::Specific::buildDirRuntime, QString(), builddir);
}

} // namespace

namespace CMake
{

KDevelop::Path::List resolveSystemDirs(KDevelop::IProject* project, const QStringList& dirs)
{
    const KDevelop::Path buildDir(CMake::currentBuildDir(project));
    const KDevelop::Path installDir(CMake::currentInstallDir(project));

    KDevelop::Path::List newList;
    newList.reserve(dirs.size());
    for (const QString& s : dirs) {
        KDevelop::Path dir;
        if(s.startsWith(QLatin1String("#[bin_dir]")))
        {
            dir = KDevelop::Path(buildDir, s);
        }
        else if(s.startsWith(QLatin1String("#[install_dir]")))
        {
            dir = KDevelop::Path(installDir, s);
        }
        else
        {
            dir = KDevelop::Path(s);
        }

//         qCDebug(CMAKE) << "resolved" << s << "to" << d;

        if (!newList.contains(dir))
        {
            newList.append(dir);
        }
    }
    return newList;
}

///NOTE: when you change this, update @c defaultConfigure in cmakemanagertest.cpp
bool checkForNeedingConfigure( KDevelop::IProject* project )
{
    auto currentRuntime = ICore::self()->runtimeController()->currentRuntime();
    const QString currentRuntimeName = currentRuntime->name();
    const KDevelop::Path builddir = currentBuildDir(project);
    const bool isValid = (buildDirRuntime(project, -1) == currentRuntimeName || buildDirRuntime(project, -1).isEmpty()) && builddir.isValid();

    if( !isValid )
    {
        auto addBuildDir = [project](const KDevelop::Path& buildFolder, const KDevelop::Path& installPrefix, const QString &extraArguments, const QString &buildType, const KDevelop::Path &cmakeExecutable){
            int addedBuildDirIndex = buildDirCount( project ); // old count is the new index

            // Initialize the kconfig items with the values from the dialog, this ensures the settings
            // end up in the config file once the changes are saved
            qCDebug(CMAKE) << "adding to cmake config: new builddir index" << addedBuildDirIndex;
            qCDebug(CMAKE) << "adding to cmake config: builddir path " << buildFolder;
            qCDebug(CMAKE) << "adding to cmake config: installdir " << installPrefix;
            qCDebug(CMAKE) << "adding to cmake config: extra args" << extraArguments;
            qCDebug(CMAKE) << "adding to cmake config: build type " << buildType;
            qCDebug(CMAKE) << "adding to cmake config: cmake executable " << cmakeExecutable;
            qCDebug(CMAKE) << "adding to cmake config: environment <null>";
            CMake::setBuildDirCount( project, addedBuildDirIndex + 1 );
            CMake::setCurrentBuildDirIndex( project, addedBuildDirIndex );
            CMake::setCurrentBuildDir( project, buildFolder );
            CMake::setCurrentInstallDir( project, installPrefix );
            CMake::setCurrentExtraArguments( project, extraArguments );
            CMake::setCurrentBuildType( project, buildType );
            CMake::setCurrentCMakeExecutable(project, cmakeExecutable );
            CMake::setCurrentEnvironment( project, QString() );
        };

        if (!currentRuntime->buildPath().isEmpty()) {
            const Path newBuilddir(currentRuntime->buildPath(), QLatin1String("build-") + currentRuntimeName + project->name());
            const Path installPath(QString::fromUtf8(currentRuntime->getenv("KDEV_DEFAULT_INSTALL_PREFIX")));

            addBuildDir(newBuilddir, installPath, {}, QStringLiteral("Debug"), {});
            setBuildDirRuntime( project, currentRuntimeName );
            return true;
        }

        CMakeBuildDirChooser bd;
        bd.setProject( project );
        const auto builddirs = CMake::allBuildDirs(project);
        bd.setAlreadyUsed( builddirs );
        bd.setShowAvailableBuildDirs(!builddirs.isEmpty());
        bd.setCMakeExecutable(currentCMakeExecutable(project));

        if( !bd.exec() )
        {
            return false;
        }

        if (bd.reuseBuilddir())
        {
            CMake::setCurrentBuildDirIndex( project, bd.alreadyUsedIndex() );
        }
        else
        {
            addBuildDir(bd.buildFolder(), bd.installPrefix(), bd.extraArguments(), bd.buildType(), bd.cmakeExecutable());
        }
        setBuildDirRuntime( project, currentRuntimeName );

        return true;
    } else if( !QFile::exists( KDevelop::Path(builddir, QStringLiteral("CMakeCache.txt")).toLocalFile() ) ||
                //TODO: maybe we could use the builder for that?
               !(QFile::exists( KDevelop::Path(builddir, QStringLiteral("Makefile")).toLocalFile() ) ||
                    QFile::exists( KDevelop::Path(builddir, QStringLiteral("build.ninja")).toLocalFile() ) ) )
    {
        // User entered information already, but cmake hasn't actually been run yet.
        setBuildDirRuntime( project, currentRuntimeName );
        return true;
    }
    setBuildDirRuntime( project, currentRuntimeName );
    return false;
}

QHash<KDevelop::Path, QStringList> enumerateTargets(const KDevelop::Path& targetsFilePath, const QString& sourceDir, const KDevelop::Path &buildDir)
{
    const QString buildPath = buildDir.toLocalFile();
    QHash<KDevelop::Path, QStringList> targets;
    QFile targetsFile(targetsFilePath.toLocalFile());
    if (!targetsFile.open(QIODevice::ReadOnly)) {
        qCDebug(CMAKE) << "Couldn't find the Targets file in" << targetsFile.fileName();
    }

    QTextStream targetsFileStream(&targetsFile);
    const QRegularExpression rx(QStringLiteral("^(.*)/CMakeFiles/(.*).dir$"));
    while (!targetsFileStream.atEnd()) {
        const QString line = targetsFileStream.readLine();
        auto match = rx.match(line);
        if (!match.isValid())
            qCDebug(CMAKE) << "invalid match for" << line;
        const QString sourcePath = match.captured(1).replace(buildPath, sourceDir);
        targets[KDevelop::Path(sourcePath)].append(match.captured(2));
    }
    return targets;
}

KDevelop::Path projectRoot(KDevelop::IProject* project)
{
    if (!project) {
        return {};
    }

    return project->path().cd(CMake::projectRootRelative(project));
}

KDevelop::Path currentBuildDir( KDevelop::IProject* project, int builddir )
{
    return KDevelop::Path(readBuildDirParameter( project, Config::Specific::buildDirPathKey, QString(), builddir ));
}

KDevelop::Path commandsFile(KDevelop::IProject* project)
{
    auto currentBuildDir = CMake::currentBuildDir(project);
    if (currentBuildDir.isEmpty()) {
        return {};
    }

    return KDevelop::Path(currentBuildDir, QStringLiteral("compile_commands.json"));
}

KDevelop::Path targetDirectoriesFile(KDevelop::IProject* project)
{
    auto currentBuildDir = CMake::currentBuildDir(project);
    if (currentBuildDir.isEmpty()) {
        return {};
    }

    return KDevelop::Path(currentBuildDir, QStringLiteral("CMakeFiles/TargetDirectories.txt"));
}

QString currentBuildType( KDevelop::IProject* project, int builddir )
{
    return readBuildDirParameter( project, Config::Specific::cmakeBuildTypeKey, QStringLiteral("Release"), builddir );
}

QString findExecutable()
{
    auto cmake = QStandardPaths::findExecutable(QStringLiteral("cmake"));
#ifdef Q_OS_WIN
    if (cmake.isEmpty())
        cmake = QStandardPaths::findExecutable(QStringLiteral("cmake"), {
            QStringLiteral("C:\\Program Files (x86)\\CMake\\bin"),
            QStringLiteral("C:\\Program Files\\CMake\\bin"),
            QStringLiteral("C:\\Program Files (x86)\\CMake 2.8\\bin"),
            QStringLiteral("C:\\Program Files\\CMake 2.8\\bin")});
#endif
    return cmake;
}

QString cmakeExecutableVersion(const QString& cmakeExecutable)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    p.start(cmakeExecutable, {QStringLiteral("--version")});
    if (!p.waitForFinished()) {
        qCWarning(CMAKE) << "failed to read cmake version for executable" << cmakeExecutable << p.errorString();
        return {};
    }

    static const QRegularExpression pattern(QStringLiteral("cmake version (\\d\\.\\d+(?:\\.\\d+)?).*"));
    const auto output = QString::fromLocal8Bit(p.readAll());
    const auto match = pattern.match(output);
    if (!match.hasMatch()) {
        qCWarning(CMAKE) << "failed to read cmake version for executable" << cmakeExecutable << output;
        return {};
    }

    const auto version = match.captured(1);
    qCDebug(CMAKE) << "cmake version for executable" << cmakeExecutable << version;
    return version;
}

KDevelop::Path currentCMakeExecutable(KDevelop::IProject* project, int builddir)
{
    auto defaultCMakeExecutable = CMakeBuilderSettings::self()->cmakeExecutable().toLocalFile();

    if (!QFileInfo::exists(ICore::self()->runtimeController()->currentRuntime()->pathInHost(KDevelop::Path(defaultCMakeExecutable)).toLocalFile()))
        defaultCMakeExecutable = CMake::findExecutable();

    if (project) {
        // check for "CMake Executable" but for now also "CMake Binary", falling back to the default.
        auto projectCMakeExecutable = readBuildDirParameter( project, Config::Specific::cmakeExecutableKey,
            readBuildDirParameter( project, Config::Specific::cmakeBinaryKey, defaultCMakeExecutable, builddir),
            builddir );
        if (projectCMakeExecutable != defaultCMakeExecutable) {
            QFileInfo info(projectCMakeExecutable);
            if (!info.isExecutable()) {
                projectCMakeExecutable = defaultCMakeExecutable;
            }
        }
        return KDevelop::Path(projectCMakeExecutable);
    }
    return KDevelop::Path(defaultCMakeExecutable);
}

KDevelop::Path currentInstallDir( KDevelop::IProject* project, int builddir )
{
    return KDevelop::Path(readBuildDirParameter( project, Config::Specific::cmakeInstallDirKey, QString(), builddir ));
}

QString projectRootRelative( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry( Config::Old::projectRootRelativeKey, "." );
}

bool hasProjectRootRelative(KDevelop::IProject* project)
{
    return baseGroup(project).hasKey( Config::Old::projectRootRelativeKey );
}

QString currentExtraArguments( KDevelop::IProject* project, int builddir )
{
    return readBuildDirParameter( project, Config::Specific::cmakeArgumentsKey, QString(), builddir );
}

void setCurrentInstallDir( KDevelop::IProject* project, const KDevelop::Path& path )
{
    writeBuildDirParameter( project, Config::Specific::cmakeInstallDirKey, path.toLocalFile() );
}

void setCurrentBuildType( KDevelop::IProject* project, const QString& type )
{
    writeBuildDirParameter( project, Config::Specific::cmakeBuildTypeKey, type );
}

void setCurrentCMakeExecutable(KDevelop::IProject* project, const KDevelop::Path& path)
{
    // maintain compatibility with older versions for now
    writeBuildDirParameter(project, Config::Specific::cmakeBinaryKey, path.toLocalFile());
    writeBuildDirParameter(project, Config::Specific::cmakeExecutableKey, path.toLocalFile());
}

void setCurrentBuildDir( KDevelop::IProject* project, const KDevelop::Path& path )
{
    writeBuildDirParameter( project, Config::Specific::buildDirPathKey, path.toLocalFile() );
}

void setProjectRootRelative( KDevelop::IProject* project, const QString& relative)
{
    writeProjectBaseParameter( project, Config::Old::projectRootRelativeKey, relative );
}

void setCurrentExtraArguments( KDevelop::IProject* project, const QString& string)
{
    writeBuildDirParameter( project, Config::Specific::cmakeArgumentsKey, string );
}

QString currentEnvironment(KDevelop::IProject* project, int builddir)
{
    return readBuildDirParameter( project, Config::Specific::cmakeEnvironmentKey, QString(), builddir );
}

int currentBuildDirIndex( KDevelop::IProject* project )
{
    KConfigGroup baseGrp = baseGroup(project);

    if ( baseGrp.hasKey( Config::buildDirOverrideIndexKey ) )
        return baseGrp.readEntry<int>( Config::buildDirOverrideIndexKey, -1 );

    else if (baseGrp.hasKey(Config::buildDirIndexKey()))
        return baseGrp.readEntry<int>( Config::buildDirIndexKey(), -1 );
    else
        return baseGrp.readEntry<int>(Config::globalBuildDirIndexKey(), -1); // backwards compatibility
}

void setCurrentBuildDirIndex( KDevelop::IProject* project, int buildDirIndex )
{
    writeProjectBaseParameter( project, Config::buildDirIndexKey(), QString::number (buildDirIndex) );
}

void setCurrentEnvironment( KDevelop::IProject* project, const QString& environment )
{
    writeBuildDirParameter( project, Config::Specific::cmakeEnvironmentKey, environment );
}

void initBuildDirConfig( KDevelop::IProject* project )
{
    int buildDirIndex = currentBuildDirIndex( project );
    if (buildDirCount(project) <= buildDirIndex )
        setBuildDirCount( project, buildDirIndex + 1 );
}

int buildDirCount( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry<int>( Config::buildDirCountKey, 0 );
}

void setBuildDirCount( KDevelop::IProject* project, int count )
{
    writeProjectBaseParameter( project, Config::buildDirCountKey, QString::number(count) );
}

void removeBuildDirConfig( KDevelop::IProject* project )
{
    int buildDirIndex = currentBuildDirIndex( project );
    if ( !buildDirGroupExists( project, buildDirIndex ) )
    {
        qCWarning(CMAKE) << "build directory config" << buildDirIndex << "to be removed but does not exist";
        return;
    }

    int bdCount = buildDirCount(project);
    setBuildDirCount( project, bdCount - 1 );
    removeOverrideBuildDirIndex( project );
    setCurrentBuildDirIndex( project, -1 );

    // move (rename) the upper config groups to keep the numbering
    // if there's nothing to move, just delete the group physically
    if (buildDirIndex + 1 == bdCount)
        buildDirGroup( project, buildDirIndex ).deleteGroup();

    else for (int i = buildDirIndex + 1; i < bdCount; ++i)
    {
        KConfigGroup src = buildDirGroup( project, i );
        KConfigGroup dest = buildDirGroup( project, i - 1 );
        dest.deleteGroup();
        src.copyTo(&dest);
        src.deleteGroup();
    }
}

QHash<QString, QString> readCacheValues(const KDevelop::Path& cmakeCachePath, QSet<QString> variables)
{
    QHash<QString, QString> ret;
    QFile file(cmakeCachePath.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(CMAKE) << "couldn't open CMakeCache.txt" << cmakeCachePath;
        return ret;
    }

    QTextStream in(&file);
    while (!in.atEnd() && !variables.isEmpty())
    {
        QString line = in.readLine().trimmed();
        if(!line.isEmpty() && line[0].isLetter())
        {
            CacheLine c;
            c.readLine(line);

            if(!c.isCorrect())
                continue;

            if (variables.remove(c.name())) {
                ret[c.name()] = c.value();
            }
        }
    }
    return ret;
}

void updateConfig( KDevelop::IProject* project, int buildDirIndex)
{
    if (buildDirIndex < 0)
        return;

    KConfigGroup buildDirGrp = buildDirGroup( project, buildDirIndex );
    const KDevelop::Path builddir(buildDirGrp.readEntry( Config::Specific::buildDirPathKey, QString() ));
    const KDevelop::Path cacheFilePath( builddir, QStringLiteral("CMakeCache.txt"));

    const QMap<QString, QLatin1String> keys = {
        { QStringLiteral("CMAKE_COMMAND"), Config::Specific::cmakeExecutableKey },
        { QStringLiteral("CMAKE_INSTALL_PREFIX"), Config::Specific::cmakeInstallDirKey },
        { QStringLiteral("CMAKE_BUILD_TYPE"), Config::Specific::cmakeBuildTypeKey }
    };

    const QSet<QString> variables(keys.keyBegin(), keys.keyEnd());
    const QHash<QString, QString> cacheValues = readCacheValues(cacheFilePath, variables);
    for(auto it = cacheValues.constBegin(), itEnd = cacheValues.constEnd(); it!=itEnd; ++it) {
        const QLatin1String key = keys.value(it.key());
        Q_ASSERT(!key.isEmpty());

        // Use cache only when the config value is not set. Without this check we will always
        // overwrite values provided by the user in config dialog.
        if (buildDirGrp.readEntry(key).isEmpty() && !it.value().isEmpty())
        {
            buildDirGrp.writeEntry( key, it.value() );
        }
    }
}

void setOverrideBuildDirIndex( KDevelop::IProject* project, int overrideBuildDirIndex )
{
    writeProjectBaseParameter( project, Config::buildDirOverrideIndexKey, QString::number(overrideBuildDirIndex) );
}

void removeOverrideBuildDirIndex( KDevelop::IProject* project, bool writeToMainIndex )
{
    KConfigGroup baseGrp = baseGroup(project);

    if( !baseGrp.hasKey(Config::buildDirOverrideIndexKey) )
        return;
    if( writeToMainIndex )
        baseGrp.writeEntry( Config::buildDirIndexKey(), baseGrp.readEntry(Config::buildDirOverrideIndexKey) );

    baseGrp.deleteEntry(Config::buildDirOverrideIndexKey);
}

ICMakeDocumentation* cmakeDocumentation()
{
    return KDevelop::ICore::self()->pluginController()->extensionForPlugin<ICMakeDocumentation>(QStringLiteral("org.kdevelop.ICMakeDocumentation"));
}

QStringList allBuildDirs(KDevelop::IProject* project)
{
    QStringList result;
    int bdCount = buildDirCount(project);
    result.reserve(bdCount);
    for (int i = 0; i < bdCount; ++i)
        result += buildDirGroup( project, i ).readEntry( Config::Specific::buildDirPathKey );
    return result;
}

QString executeProcess(const QString& execName, const QStringList& args)
{
    Q_ASSERT(!execName.isEmpty());
    qCDebug(CMAKE) << "Executing:" << execName << "::" << args;

    QProcess p;
    QTemporaryDir tmp(QStringLiteral("kdevcmakemanager"));
    p.setWorkingDirectory( tmp.path() );
    p.start(execName, args, QIODevice::ReadOnly);

    if(!p.waitForFinished())
    {
        qCDebug(CMAKE) << "failed to execute:" << execName << args << p.exitStatus() << p.readAllStandardError();
    }

    QByteArray b = p.readAllStandardOutput();
    QString t = QString::fromUtf8(b.trimmed());
    return t;
}

QStringList supportedGenerators()
{
    QStringList generatorNames;

    bool hasNinja = ICore::self() && ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("KDevNinjaBuilder"));
    if (hasNinja)
        generatorNames << QStringLiteral("Ninja");

#ifdef Q_OS_WIN
    // Visual Studio solution is the standard generator under windows, but we don't want to use
    // the VS IDE, so we need nmake makefiles
    generatorNames << QStringLiteral("NMake Makefiles") << QStringLiteral("MinGW Makefiles");
#endif
    generatorNames << QStringLiteral("Unix Makefiles");

    return generatorNames;
}

QString defaultGenerator()
{
    const QStringList generatorNames = supportedGenerators();

    QString defGen = generatorNames.value(CMakeBuilderSettings::self()->generator());
    if (defGen.isEmpty())
    {
        qCWarning(CMAKE) << "Couldn't find builder with index " << CMakeBuilderSettings::self()->generator()
                   << ", defaulting to 0";
        CMakeBuilderSettings::self()->setGenerator(0);
        defGen = generatorNames.at(0);
    }
    return defGen;
}

QVector<CMakeTest> importTestSuites(const Path &buildDir, const QString &cmakeTestFileName)
{
    const auto cmakeTestFile = Path(buildDir, cmakeTestFileName).toLocalFile()  ;
    const auto contents = CMakeListsParser::readCMakeFile(cmakeTestFile);
    
    QVector<CMakeTest> tests;
    for (const auto& entry: contents) {
        if (entry.name == QLatin1String("add_test")) {
            auto args = entry.arguments;
            CMakeTest test;
            test.name = args.takeFirst().value;
            test.executable = args.takeFirst().value;
            test.arguments = kTransform<QStringList>(args, [](const CMakeFunctionArgument& arg) { return arg.value; });
            tests += test;
        } else if (entry.name == QLatin1String("subdirs")) {
            tests += importTestSuites(Path(buildDir, entry.arguments.first().value));
        } else if (entry.name == QLatin1String("include")) {
            // Include directive points directly to a .cmake file hosting the tests
            tests += importTestSuites(Path(buildDir, entry.arguments.first().value), QString());
        } else if (entry.name == QLatin1String("set_tests_properties")) {
            if(entry.arguments.count() < 4 || entry.arguments.count() % 2) {
                qCWarning(CMAKE) << "found set_tests_properties() with unexpected number of arguments:"
                                 << entry.arguments.count();
                continue;
            }
            if (tests.isEmpty() || entry.arguments.first().value != tests.last().name) {
                qCWarning(CMAKE) << "found set_tests_properties(" << entry.arguments.first().value
                                 << " ...), but expected test " << tests.last().name;
                continue;
            }
            if (entry.arguments[1].value != QLatin1String("PROPERTIES")) {
                qCWarning(CMAKE) << "found set_tests_properties(" << entry.arguments.first().value
                                 << entry.arguments.at(1).value << "...), but expected PROPERTIES as second argument";
                continue;
            }
            CMakeTest &test = tests.last();
            for (int i = 2; i < entry.arguments.count(); i += 2)
                test.properties[entry.arguments[i].value] = entry.arguments[i + 1].value;
        }
    }

    return tests;
}

QVector<CMakeTest> importTestSuites(const Path &buildDir) {
    return importTestSuites(buildDir, QStringLiteral("CTestTestfile.cmake"));
}

}
