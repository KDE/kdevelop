/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "main.h"

#include <shell/core.h>
#include <shell/shellextension.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/definitions.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchaindumper.h>
#include <language/duchain/dumpdotgraph.h>
#include <language/duchain/problem.h>
#include <language/duchain/persistentsymboltable.h>

#include <interfaces/ilanguagecontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDirIterator>
#include <QStringList>
#include <QTimer>

#include <cstdio>

#include <KAboutData>
#include <KLocalizedString>

bool verbose = false, warnings = false;

using namespace KDevelop;

void messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);

    switch (type) {
    case QtInfoMsg:     // fall-through
    case QtDebugMsg:
        if (verbose)
            std::cerr << qPrintable(msg) << std::endl;
        break;
    case QtWarningMsg:
        if (warnings)
            std::cerr << qPrintable(msg) << std::endl;
        break;
    case QtCriticalMsg:
        std::cerr << qPrintable(msg) << std::endl;
        break;
    case QtFatalMsg:
        std::cerr << qPrintable(msg) << std::endl;
        abort();
    }
}

Manager::Manager(QCommandLineParser* args) : m_total(0)
    , m_args(args)
    , m_allFilesAdded(0)
{
}

void Manager::init()
{
    if (m_args->positionalArguments().isEmpty()) {
        std::cerr << "Need file or directory to duchainify" << std::endl;
        QCoreApplication::exit(1);
    }

    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;
    if (m_args->isSet(QStringLiteral("features"))) {
        QString featuresStr = m_args->value(QStringLiteral("features"));
        if (featuresStr == QLatin1String("visible-declarations")) {
            features = TopDUContext::VisibleDeclarationsAndContexts;
        } else if (featuresStr == QLatin1String("all-declarations")) {
            features = TopDUContext::AllDeclarationsAndContexts;
        } else if (featuresStr == QLatin1String("all-declarations-and-uses")) {
            features = TopDUContext::AllDeclarationsContextsAndUses;
        } else if (featuresStr == QLatin1String("all-declarations-and-uses-and-AST")) {
            features = TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST;
        } else if (featuresStr == QLatin1String("empty")) {
            features = TopDUContext::Empty;
        } else if (featuresStr == QLatin1String("simplified-visible-declarations")) {
            features = TopDUContext::SimplifiedVisibleDeclarationsAndContexts;
        } else {
            std::cerr << "Wrong feature-string given\n";
            QCoreApplication::exit(2);
            return;
        }
    }
    if (m_args->isSet(QStringLiteral("force-update")))
        features |= TopDUContext::ForceUpdate;
    if (m_args->isSet(QStringLiteral("force-update-recursive")))
        features |= TopDUContext::ForceUpdateRecursive;

    if (m_args->isSet(QStringLiteral("threads"))) {
        bool ok = false;
        int count = m_args->value(QStringLiteral("threads")).toInt(&ok);
        ICore::self()->languageController()->backgroundParser()->setThreadCount(count);
        if (!ok) {
            std::cerr << "bad thread count\n";
            QCoreApplication::exit(3);
            return;
        }
    }

    // quit when everything is done
    // background parser emits hideProgress() signal in two situations:
    // when everything is done and when bgparser is suspended
    // later doesn't happen in duchain, so just rely on hideProgress()
    // and quit when it's emitted
    connect(
        ICore::self()->languageController()->backgroundParser(), &BackgroundParser::hideProgress, this,
        [this]() {
            if (ICore::self()->languageController()->backgroundParser()->isIdle())
                QTimer::singleShot(0, this, &Manager::finish);
        });

    const auto files = m_args->positionalArguments();
    for (const auto& file : files) {
        addToBackgroundParser(file, features);
    }

    m_allFilesAdded = 1;

    if (m_total) {
        std::cerr << "Added " << m_total << " files to the background parser" << std::endl;
        const int threads = ICore::self()->languageController()->backgroundParser()->threadCount();
        std::cerr << "parsing with " << threads << " threads" << std::endl;
        ICore::self()->languageController()->backgroundParser()->parseDocuments();
    } else {
        std::cerr << "no files added to the background parser" << std::endl;
        QCoreApplication::exit(0);
        return;
    }
}

void Manager::updateReady(const IndexedString& url, const ReferencedTopDUContext& topContext)
{
    qDebug() << "finished" << url.toUrl().toLocalFile() << "success: " << ( bool )topContext;

    m_waiting.remove(url.toUrl());

    std::cerr << "processed " << (m_total - m_waiting.size()) << " out of " << m_total << "\n";
    dump(topContext);
}

void Manager::dump(const ReferencedTopDUContext& topContext)
{
    if (!topContext) {
        return;
    }

    QTextStream stream(stdout);

    std::cerr << "\n";

    if (m_args->isSet(QStringLiteral("dump-definitions"))) {
        DUChainReadLocker lock;
        std::cerr << "Definitions:" << std::endl;
        DUChain::definitions()->dump(stream);
        std::cerr << std::endl;
    }

    if (m_args->isSet(QStringLiteral("dump-symboltable"))) {
        DUChainReadLocker lock;
        std::cerr << "PersistentSymbolTable:" << std::endl;
        PersistentSymbolTable::self().dump(stream);
        std::cerr << std::endl;
    }

    DUChainDumper::Features features;
    if (m_args->isSet(QStringLiteral("dump-context"))) {
        features |= DUChainDumper::DumpContext;
    }
    if (m_args->isSet(QStringLiteral("dump-errors"))) {
        features |= DUChainDumper::DumpProblems;
    }

    if (auto depth = m_args->value(QStringLiteral("dump-depth")).toInt()) {
        DUChainReadLocker lock;
        std::cerr << "Context:" << std::endl;
        DUChainDumper dumpChain(features);
        dumpChain.dump(topContext, depth);
    }

    if (m_args->isSet(QStringLiteral("dump-graph"))) {
        DUChainReadLocker lock;
        DumpDotGraph dumpGraph;
        const QString dotOutput = dumpGraph.dotGraph(topContext);
        std::cout << qPrintable(dotOutput) << std::endl;
    }

    const auto dumpImportedErrors = m_args->isSet(QStringLiteral("dump-imported-errors"));
    const auto dumpParentContexts = m_args->isSet(QStringLiteral("dump-parent-contexts"));
    if (dumpImportedErrors || dumpParentContexts) {
        DUChainReadLocker lock;
        const auto imports = topContext->importedParentContexts();
        for (const auto& import : imports) {
            auto top = dynamic_cast<TopDUContext*>(import.indexedContext().context());
            if (!top || top == topContext) {
                continue;
            }

            if (dumpParentContexts) {
                dump(top);
            } else if (!top->problems().isEmpty()) {
                DUChainDumper dumpChain(DUChainDumper::DumpProblems);
                dumpChain.dump(top, 0);
            }
        }
    }
}

void Manager::addToBackgroundParser(const QString& path, TopDUContext::Features features)
{
    QFileInfo info(path);

    if (info.isFile()) {
        qDebug() << "adding file" << path;
        QUrl pathUrl = QUrl::fromLocalFile(info.canonicalFilePath());

        m_waiting << pathUrl;
        ++m_total;

        KDevelop::DUChain::self()->updateContextForUrl(KDevelop::IndexedString(pathUrl), features, this);

    } else if (info.isDir()) {
        QDirIterator contents(path);
        while (contents.hasNext()) {
            QString newPath = contents.next();
            if (!newPath.endsWith(QLatin1Char('.')))
                addToBackgroundParser(newPath, features);
        }
    }
}

QSet<QUrl> Manager::waiting()
{
    return m_waiting;
}

void Manager::finish()
{
    std::cerr << "ready" << std::endl;
    QCoreApplication::quit();
}

using namespace KDevelop;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("duchainify"), i18n("duchainify"),
        QStringLiteral("1"), i18n("DUChain builder application"), KAboutLicense::GPL,
        i18n("(c) 2009 David Nolden"), QString(), QStringLiteral("https://www.kdevelop.org/"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("paths"), i18n("file or directory"), QStringLiteral("[PATH...]"));

    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("w"), QStringLiteral("warnings")},
                                        i18n("Show warnings")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("V"), QStringLiteral("verbose")},
                                        i18n("Show warnings and debug output")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("u"), QStringLiteral("force-update")},
                                        i18n("Enforce an update of the top-contexts corresponding to the given files")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("r"), QStringLiteral(
                                                        "force-update-recursive")},
                                        i18n(
                                            "Enforce an update of the top-contexts corresponding to the given files and all included files")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("t"), QStringLiteral("threads")},
                                        i18n("Number of threads to use"), QStringLiteral("count")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("f"), QStringLiteral("features")},
                                        i18n(
                                            "Features to build. Options: empty, simplified-visible-declarations, visible-declarations (default), all-declarations, all-declarations-and-uses, all-declarations-and-uses-and-AST"),
                                        QStringLiteral("features")});

    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-context")},
                                        i18n("Print complete Definition-Use Chain on successful parse")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-parent-contexts")},
                                        i18n("Print parent contexts when dumping contexts")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-definitions")},
                                        i18n("Print complete DUChain Definitions repository on successful parse")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-symboltable")},
                                        i18n(
                                            "Print complete DUChain PersistentSymbolTable repository on successful parse")});
    parser.addOption(
        QCommandLineOption { QStringList { QStringLiteral("dump-depth") },
                             i18n("Number defining the maximum depth where declaration details are printed"),
                             QStringLiteral("depth"), QStringLiteral("100") });
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-graph")},
                                        i18n("Dump DUChain graph (in .dot format)")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("d"), QStringLiteral("dump-errors")},
                                        i18n("Print problems encountered during parsing")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("dump-imported-errors")},
                                        i18n("Recursively dump errors from imported contexts.")});

    parser.process(app);

    aboutData.processCommandLine(&parser);

    verbose = parser.isSet(QStringLiteral("verbose"));
    warnings = parser.isSet(QStringLiteral("warnings"));
    qInstallMessageHandler(messageOutput);

    AutoTestShell::init();
    TestCore::initialize(Core::NoUi, QStringLiteral("duchainify"));
    Manager manager(&parser);

    QTimer::singleShot(0, &manager, &Manager::init);
    int ret = app.exec();

    TestCore::shutdown();

    return ret;
}

#include "moc_main.cpp"
