#include "tools_part.h"

#include <qfile.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kprocess.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/selectioninterface.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevappfrontend.h"
#include "kdevplugininfo.h"
#include "urlutil.h"

#include "toolsconfig.h"
#include "toolsconfigwidget.h"


static const KDevPluginInfo data("kdevtools");
K_EXPORT_COMPONENT_FACTORY( libkdevtools, ToolsFactory( data ) )

ToolsPart::ToolsPart(QObject *parent, const char *name, const QStringList &)
	: KDevPlugin( &data, parent, name ? name : "ToolsPart")
{
  setInstance(ToolsFactory::instance());

  setXMLFile("kdevpart_tools.rc");

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

  connect(core(), SIGNAL(coreInitialized()), this, SLOT(updateMenu()));

  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
           this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

  // Apparently action lists can only be plugged after the
  // xmlgui client has been registered
  QTimer::singleShot(0, this, SLOT(updateToolsMenu()));
}


ToolsPart::~ToolsPart()
{
}


void ToolsPart::configWidget(KDialogBase *dlg)
{
	QVBox *vbox = dlg->addVBoxPage( i18n("Tools Menu"), i18n("Tools Menu"), BarIcon( info()->icon(), KIcon::SizeMedium) );
  ToolsConfig *w = new ToolsConfig(vbox, "tools config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
  connect(dlg, SIGNAL(destroyed()), this, SLOT(updateMenu()));

  vbox = dlg->addVBoxPage(i18n("External Tools"), i18n("External Tools"), BarIcon( info()->icon(), KIcon::SizeMedium) );
  ToolsConfigWidget *w2 = new ToolsConfigWidget(vbox, "tools config widget");
  connect(dlg, SIGNAL(okClicked()), w2, SLOT(accept()));
  connect(dlg, SIGNAL(destroyed()), this, SLOT(updateToolsMenu()));
}



void ToolsPart::updateMenu()
{
  QPtrList<KAction> actions;

  unplugActionList("tools_list");

  KConfig *config = ToolsFactory::instance()->config();
  config->setGroup("Tools");

  QStringList list = config->readListEntry("Tools");
  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
	{
	  QString name = *it;

	  KDesktopFile df(name, true);
	  if (df.readName().isNull())
		continue;

	  KAction *action = new KAction(df.readName(), df.readIcon(), 0,
                                        this, SLOT(slotToolActivated()), (QObject*)0, name.latin1());
	  actions.append(action);
	}

  plugActionList("tools_list", actions);
}


void ToolsPart::slotToolActivated()
{
  QString df = sender()->name();
  kapp->startServiceByDesktopPath(df);
}


// Duplicated from abbrev part. This really should be in
// the editor interface!
static QString currentWord(KTextEditor::EditInterface *editiface,
                           KTextEditor::ViewCursorInterface *cursoriface)
{
    uint line, col;
    cursoriface->cursorPositionReal(&line, &col);
    QString str = editiface->textLine(line);
    int i;
    for (i = col-1; i >= 0; --i)
        if (!str[i].isLetter())
            break;

    return str.mid(i+1, col-i-1);
}


void ToolsPart::startCommand(QString cmdline, bool captured, QString fileName)
{
    KParts::Part *part = partController()->activePart();
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(part);
    KTextEditor::SelectionInterface *selectionIface
        = dynamic_cast<KTextEditor::SelectionInterface*>(part);
    KTextEditor::EditInterface *editIface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(part);

    if (fileName.isNull() && rwpart)
        fileName = rwpart->url().path();
    
    QString projectDirectory;
    if (project())
        projectDirectory = project()->projectDirectory();
    
    QString selection;
    if (selectionIface)
        selection = KShellProcess::quote(selectionIface->selection());

    QString word;
    if (editIface && cursorIface)
        word = KShellProcess::quote(currentWord(editIface, cursorIface));
    
    // This should really be checked before inserting into the popup
    if (cmdline.contains("%D") && projectDirectory.isNull())
        return;
    cmdline.replace(QRegExp("%D"), projectDirectory);
    
    if (cmdline.contains("%S") && fileName.isNull())
        return;
    cmdline.replace(QRegExp("%S"), fileName);

    if (cmdline.contains("%T") && selection.isNull())
        return;
    cmdline.replace(QRegExp("%T"), selection);

    if (cmdline.contains("%W") && word.isNull())
        return;
    cmdline.replace(QRegExp("%W"), word);

    if (captured)
       if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
            appFrontend->startAppCommand(QString::QString(), cmdline, false);
    else {
        KShellProcess proc;
        proc << cmdline;
        proc.start(KProcess::DontCare, KProcess::NoCommunication);
    }
}


void ToolsPart::updateToolsMenu()
{
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("External Tools");
    QStringList l = config->readListEntry("Tool Menu");

    QPtrList<KAction> actions;
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        QString menutext = *it;
        KConfig *config = ToolsFactory::instance()->config();
        config->setGroup("Tool Menu " + menutext);
        bool isdesktopfile = config->readBoolEntry("DesktopFile");
        KAction *action = new KAction(*it, 0,
                                      this, SLOT(toolsMenuActivated()),
                                      (QObject*) 0, menutext.utf8());
        if (isdesktopfile) {
            KDesktopFile df(config->readPathEntry("CommandLine"));
            action->setIcon(df.readIcon());
        }
        actions.append(action);
    }

    unplugActionList("tools2_list");
    plugActionList("tools2_list", actions);
}


void ToolsPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType( Context::FileContext ))
        return;

    const FileContext *fcontext = static_cast<const FileContext*>(context);
    m_contextPopup = popup;
    m_contextFileName = fcontext->urls().first().fileName();
    
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("External Tools");
    QStringList filecontextList = config->readListEntry("File Context");

    if (URLUtil::isDirectory(m_contextFileName)) {
        QStringList l = config->readListEntry("Dir Context");
        QStringList::ConstIterator it;
        for (it = l.begin(); it != l.end(); ++it)
            popup->insertItem( (*it), this, SLOT(dirContextActivated(int)) );
    } else {
        QStringList l = config->readListEntry("File Context");
        QStringList::ConstIterator it;
        for (it = l.begin(); it != l.end(); ++it)
            popup->insertItem( (*it), this, SLOT(fileContextActivated(int))  );
    }
}


void ToolsPart::toolsMenuActivated()
{
    QString menutext = QString::fromUtf8(sender()->name());
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("Tool Menu " + menutext);
    QString cmdline = config->readPathEntry("CommandLine");
    bool isdesktopfile = config->readBoolEntry("DesktopFile");
    bool captured = config->readBoolEntry("Captured");
    kdDebug() << "activating " << menutext
              << "with cmdline " << cmdline
              << "and desktopfile " << isdesktopfile << endl;
    if (isdesktopfile)
        kapp->startServiceByDesktopPath(cmdline);
    else
        startCommand(cmdline, captured, QString::null);
}


void ToolsPart::fileContextActivated(int id)
{
    QString menutext = m_contextPopup->text(id);
    
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("File Context " + menutext);
    QString cmdline = config->readPathEntry("CommandLine");
    bool captured = config->readBoolEntry("Captured");
    kdDebug() << "activating " << menutext
              << "with cmdline " << cmdline
              << " on file " << m_contextFileName << endl;
    startCommand(cmdline, captured, m_contextFileName);
}


void ToolsPart::dirContextActivated(int id)
{
    QString menutext = m_contextPopup->text(id);

    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("Dir Context " + menutext);
    QString cmdline = config->readPathEntry("CommandLine");
    bool captured = config->readBoolEntry("Captured");
    kdDebug() << "activating " << menutext
              << "with cmdline " << cmdline
              << " on directory " << m_contextFileName << endl;
    startCommand(cmdline, captured, m_contextFileName);
}

#include "tools_part.moc"
