/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <language/duchain/indexedstring.h>
#include <QMenu>
#include <QLineEdit>

Q_DECLARE_METATYPE(KDevelop::IProject*);

KIcon PlasmoidExecutionConfig::icon() const
{
    return KIcon("system-run");
}

void PlasmoidExecutionConfig::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* )
{
    bool b = blockSignals( true );
    identifier->lineEdit()->setText(cfg.readEntry("PlasmoidIdentifier", ""));
    blockSignals( b );
    
    QStringList arguments = cfg.readEntry("Arguments", QStringList());
    int idxFormFactor = arguments.indexOf("--formfactor")+1;
    if(idxFormFactor>0)
        formFactor->setCurrentIndex(formFactor->findText(arguments[idxFormFactor]));
    
    int idxTheme = arguments.indexOf("--theme")+1;
    if(idxTheme>0)
        themes->setCurrentIndex(themes->findText(arguments[idxTheme]));
}

QStringList readProcess(QProcess* p)
{
    QStringList ret;
    while(!p->atEnd()) {
        QByteArray line = p->readLine();
        int nameEnd=line.indexOf(' ');
        if(nameEnd>0) {
            ret += line.left(nameEnd);
        }
    }
    return ret;
}

PlasmoidExecutionConfig::PlasmoidExecutionConfig( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    connect( identifier->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    
    QProcess pPlasmoids;
    pPlasmoids.start("plasmoidviewer", QStringList("--list"), QIODevice::ReadOnly);
    
    QProcess pThemes;
    pThemes.start("plasmoidviewer", QStringList("--list-themes"), QIODevice::ReadOnly);
    pThemes.waitForFinished();
    pPlasmoids.waitForFinished();
    
    foreach(const QString& plasmoid, readProcess(&pPlasmoids)) {
        identifier->addItem(plasmoid);
    }
    
    themes->addItem(QString());
    foreach(const QString& theme, readProcess(&pThemes)) {
        themes->addItem(theme);
    }
}

void PlasmoidExecutionConfig::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry("PlasmoidIdentifier", identifier->lineEdit()->text());
    QStringList args;
    args += "--formfactor";
    args += formFactor->currentText();
    if(!themes->currentText().isEmpty()) {
        args += "--theme";
        args += themes->currentText();
    }
    cfg.writeEntry("Arguments", args);
}

QString PlasmoidExecutionConfig::title() const
{
    return i18n("Configure Plasmoid Execution");
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
    return "PlasmoidLauncher";
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
        return 0;
    }
    
    if( launchMode == "execute" )
    {
        return new PlasmoidExecutionJob(m_plugin, cfg);
    }
    kWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

QStringList PlasmoidLauncher::supportedModes() const
{
    return QStringList() << "execute";
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

QString PlasmoidExecutionConfigType::id() const
{
    return "PlasmoudLauncherType";
}

KIcon PlasmoidExecutionConfigType::icon() const
{
    return KIcon("plasma");
}

bool canLaunchMetadataFile(const KUrl& url)
{
    KConfig cfg(url.toLocalFile(), KConfig::SimpleConfig);
    KConfigGroup group(&cfg, "Desktop Entry");
    return group.readEntry("ServiceTypes", QString()) == "Plasma/Applet";
}

//don't bother, nobody uses this interface
bool PlasmoidExecutionConfigType::canLaunch(const KUrl& ) const
{
    return false;
}

bool PlasmoidExecutionConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    KDevelop::ProjectFolderItem* folder = item->folder();
    if(folder && folder->hasFileOrFolder("metadata.desktop")) {
        return canLaunchMetadataFile(KUrl(folder->url(), "metadata.desktop"));
    }
    return false;
}

void PlasmoidExecutionConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry("PlasmoidIdentifier", item->url().toLocalFile());
}

void PlasmoidExecutionConfigType::configureLaunchFromCmdLineArguments(KConfigGroup /*config*/, const QStringList &/*args*/) const
{}

QMenu* PlasmoidExecutionConfigType::launcherSuggestions()
{
    QList<QAction*> found;
    QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();
    foreach(KDevelop::IProject* p, projects) {
        QSet<KDevelop::IndexedString> files = p->fileSet();
        foreach(const KDevelop::IndexedString& file, files) {
            KUrl url = file.toUrl();
            if(url.fileName()=="metadata.desktop" && canLaunchMetadataFile(url)) {
                url = url.upUrl();
                KUrl relUrl = p->relativeUrl(url);
                QAction* action = new QAction(relUrl.prettyUrl(KUrl::RemoveTrailingSlash), this);
                action->setProperty("url", relUrl.toLocalFile(KUrl::RemoveTrailingSlash));
                action->setProperty("project", qVariantFromValue<KDevelop::IProject*>(p));
                connect(action, SIGNAL(triggered(bool)), SLOT(suggestionTriggered()));
                found.append(action);
            }
        }
    }
    
    QMenu *m = 0;
    if(!found.isEmpty()) {
        m = new QMenu(i18n("Plasmoids"));
        m->addActions(found);
    }
    return m;
}

void PlasmoidExecutionConfigType::suggestionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    KDevelop::IProject* p = action->property("project").value<KDevelop::IProject*>();
    QString relUrl = action->property("url").toString();
    
    QPair<QString,QString> launcher = qMakePair( launchers().at( 0 )->supportedModes().at(0), launchers().at( 0 )->id() );
    
    QString name = relUrl.mid(relUrl.lastIndexOf('/')+1);
    KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, name);
    KConfigGroup cfg = config->config();
    cfg.writeEntry("PlasmoidIdentifier", relUrl);
    emit signalAddLaunchConfiguration(config);
}
