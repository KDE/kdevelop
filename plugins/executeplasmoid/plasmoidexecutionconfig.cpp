/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"
#include "debug.h"

#include <KLocalizedString>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <serialization/indexedstring.h>
#include <util/kdevstringhandler.h>
#include <util/executecompositejob.h>
#include <util/path.h>

#include <KMessageBox>
#include <KParts/MainWindow>
#include <KConfigGroup>
#include <QMenu>
#include <QLineEdit>

QIcon PlasmoidExecutionConfig::icon() const
{
    return QIcon::fromTheme(QStringLiteral("system-run"));
}

QStringList readProcess(QProcess* p)
{
    QStringList ret;
    while(!p->atEnd()) {
        QByteArray line = p->readLine();
        int nameEnd=line.indexOf(' ');
        if(nameEnd>0) {
            ret += QString::fromUtf8(line.left(nameEnd));
        }
    }
    return ret;
}

PlasmoidExecutionConfig::PlasmoidExecutionConfig( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    connect( identifier->lineEdit(), &QLineEdit::textEdited, this, &PlasmoidExecutionConfig::changed );

    QProcess pPlasmoids;
    pPlasmoids.start(QStringLiteral("plasmoidviewer"), QStringList(QStringLiteral("--list")), QIODevice::ReadOnly);

    QProcess pThemes;
    pThemes.start(QStringLiteral("plasmoidviewer"), QStringList(QStringLiteral("--list-themes")), QIODevice::ReadOnly);
    pThemes.waitForFinished();
    pPlasmoids.waitForFinished();

    const auto plasmoidListing = readProcess(&pPlasmoids);
    for (const QString& plasmoid : plasmoidListing) {
        identifier->addItem(plasmoid);
    }

    themes->addItem(QString());
    const auto themeListing = readProcess(&pThemes);
    for (const QString& theme : themeListing) {
        themes->addItem(theme);
    }

    connect( dependencies, &KDevelop::DependenciesWidget::changed, this, &PlasmoidExecutionConfig::changed );
}

void PlasmoidExecutionConfig::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry("PlasmoidIdentifier", identifier->lineEdit()->text());
    QStringList args{
        QStringLiteral("--formfactor"),
        formFactor->currentText(),
    };
    if(!themes->currentText().isEmpty()) {
        args += QStringLiteral("--theme");
        args += themes->currentText();
    }
    cfg.writeEntry("Arguments", args);

    QVariantList deps = dependencies->dependencies();
    cfg.writeEntry( "Dependencies", KDevelop::qvariantToString( QVariant( deps ) ) );
}

void PlasmoidExecutionConfig::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* )
{
    bool b = blockSignals( true );
    identifier->lineEdit()->setText(cfg.readEntry("PlasmoidIdentifier", ""));
    blockSignals( b );

    QStringList arguments = cfg.readEntry("Arguments", QStringList());
    int idxFormFactor = arguments.indexOf(QStringLiteral("--formfactor"))+1;
    if(idxFormFactor>0)
        formFactor->setCurrentIndex(formFactor->findText(arguments[idxFormFactor]));

    int idxTheme = arguments.indexOf(QStringLiteral("--theme"))+1;
    if(idxTheme>0)
        themes->setCurrentIndex(themes->findText(arguments[idxTheme]));

    dependencies->setDependencies( KDevelop::stringToQVariant( cfg.readEntry( "Dependencies", QString() ) ).toList());
}

QString PlasmoidExecutionConfig::title() const
{
    return i18nc("@title:tab", "Configure Plasmoid Execution");
}

QList< KDevelop::LaunchConfigurationPageFactory* > PlasmoidLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString PlasmoidLauncher::description() const
{
    return i18n("Display a plasmoid");
}

QString PlasmoidLauncher::id()
{
    return QStringLiteral("PlasmoidLauncher");
}

QString PlasmoidLauncher::name() const
{
    return i18n("Plasmoid Launcher");
}

PlasmoidLauncher::PlasmoidLauncher(ExecutePlasmoidPlugin* plugin)
    : m_plugin(plugin)
{
}

KJob* PlasmoidLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return nullptr;
    }

    if( launchMode == QLatin1String("execute") )
    {
        QList<KJob*> jobs;
        if (auto* const depsJob = calculateDependencies(cfg)) {
            jobs << depsJob;
        }
        jobs << new PlasmoidExecutionJob(m_plugin, cfg);

        return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), jobs );
    }
    qCWarning(EXECUTEPLASMOID) << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return nullptr;
}

KJob* PlasmoidLauncher::calculateDependencies(KDevelop::ILaunchConfiguration* cfg)
{
    const QVariantList deps = KDevelop::stringToQVariant(cfg->config().readEntry("Dependencies", QString())).toList();
    if( !deps.isEmpty() )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QList<KDevelop::ProjectBaseItem*> items;
        for (const QVariant& dep : deps) {
            KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex( dep.toStringList() ) );
            if( item )
            {
                items << item;
            }
            else
            {
                KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                   i18n("Could not resolve the dependency: %1", dep.toString()));
            }
        }
        auto* job = new KDevelop::BuilderJob;
        job->addItems( KDevelop::BuilderJob::Install, items );
        job->updateJobName();
        return job;
    }
    return nullptr;
}

QStringList PlasmoidLauncher::supportedModes() const
{
    return QStringList() << QStringLiteral("execute");
}

KDevelop::LaunchConfigurationPage* PlasmoidPageFactory::createWidget(QWidget* parent)
{
    return new PlasmoidExecutionConfig( parent );
}

PlasmoidPageFactory::PlasmoidPageFactory()
{}

PlasmoidExecutionConfigType::PlasmoidExecutionConfigType()
{
    factoryList.append( new PlasmoidPageFactory );
}

PlasmoidExecutionConfigType::~PlasmoidExecutionConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString PlasmoidExecutionConfigType::name() const
{
    return i18n("Plasmoid Launcher");
}

QList<KDevelop::LaunchConfigurationPageFactory*> PlasmoidExecutionConfigType::configPages() const
{
    return factoryList;
}

QString PlasmoidExecutionConfigType::typeId()
{
    return QStringLiteral("PlasmoidLauncherType");
}

QIcon PlasmoidExecutionConfigType::icon() const
{
    return QIcon::fromTheme(QStringLiteral("plasma"));
}

static bool canLaunchMetadataFile(const KDevelop::Path &path)
{
    KConfig cfg(path.toLocalFile(), KConfig::SimpleConfig);
    KConfigGroup group(&cfg, QStringLiteral("Desktop Entry"));
    QStringList services = group.readEntry("ServiceTypes", group.readEntry("X-KDE-ServiceTypes", QStringList()));
    return services.contains(QStringLiteral("Plasma/Applet"));
}

//don't bother, nobody uses this interface
bool PlasmoidExecutionConfigType::canLaunch(const QUrl& ) const
{
    return false;
}

bool PlasmoidExecutionConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    KDevelop::ProjectFolderItem* folder = item->folder();
    if(folder && folder->hasFileOrFolder(QStringLiteral("metadata.desktop"))) {
        return canLaunchMetadataFile(KDevelop::Path(folder->path(), QStringLiteral("metadata.desktop")));
    }
    return false;
}

void PlasmoidExecutionConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry("PlasmoidIdentifier", item->path().toUrl().toLocalFile());
}

void PlasmoidExecutionConfigType::configureLaunchFromCmdLineArguments(KConfigGroup /*config*/, const QStringList &/*args*/) const
{}

QMenu* PlasmoidExecutionConfigType::launcherSuggestions()
{
    QList<QAction*> found;
    const QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();
    for (KDevelop::IProject* p : projects) {
        const QSet<KDevelop::IndexedString> files = p->fileSet();
        for (const KDevelop::IndexedString& file : files) {
            KDevelop::Path path(file.str());
            if (path.lastPathSegment() == QLatin1String("metadata.desktop") && canLaunchMetadataFile(path)) {
                path = path.parent();
                QString relUrl = p->path().relativePath(path);
                auto* action = new QAction(relUrl, this);
                action->setProperty("url", relUrl);
                action->setProperty("project", QVariant::fromValue<KDevelop::IProject*>(p));
                connect(action, &QAction::triggered, this, &PlasmoidExecutionConfigType::suggestionTriggered);
                found.append(action);
            }
        }
    }

    QMenu *m = nullptr;
    if(!found.isEmpty()) {
        m = new QMenu(i18nc("@title:menu", "Plasmoids"));
        m->addActions(found);
    }
    return m;
}

void PlasmoidExecutionConfigType::suggestionTriggered()
{
    auto* action = qobject_cast<QAction*>(sender());
    auto* p = action->property("project").value<KDevelop::IProject*>();
    QString relUrl = action->property("url").toString();

    KDevelop::ILauncher* launcherInstance = launchers().at( 0 );
    QPair<QString,QString> launcher = qMakePair( launcherInstance->supportedModes().at(0), launcherInstance->id() );

    QString name = relUrl.mid(relUrl.lastIndexOf(QLatin1Char('/'))+1);
    KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, name);
    KConfigGroup cfg = config->config();
    cfg.writeEntry("PlasmoidIdentifier", relUrl);
    emit signalAddLaunchConfiguration(config);
}

#include "moc_plasmoidexecutionconfig.cpp"
