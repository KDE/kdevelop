%{CPP_TEMPLATE}

#include "%{APPNAMELC}_part.h"

#include <kparts/genericfactory.h>
#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

typedef KParts::GenericFactory<%{APPNAMELC}Part> %{APPNAMELC}PartFactory;
K_EXPORT_COMPONENT_FACTORY( lib%{APPNAMELC}part, %{APPNAMELC}PartFactory );

%{APPNAMELC}Part::%{APPNAMELC}Part( QWidget *parentWidget, const char *widgetName, 
		QObject *parent, const char *name, const QStringList & /*args*/)
    : KParts::ReadWritePart(parent)
{
    // we need an instance
    setInstance( %{APPNAMELC}PartFactory::instance() );

    // this should be your custom internal widget
    m_widget = new QMultiLineEdit( parentWidget, widgetName );

    // notify the part that this is our internal widget
    setWidget(m_widget);

    // create our actions
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::save(this, SLOT(save()), actionCollection());

    // set our XML-UI resource file
    setXMLFile("%{APPNAMELC}_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);
}

%{APPNAMELC}Part::~%{APPNAMELC}Part()
{
}

KAboutData *%{APPNAMELC}Part::createAboutData()
{
        // The non-i18n name here must be the same as the directory in
        // which the part's rc file is installed
        KAboutData *aboutData = new KAboutData("%{APPNAMELC}part", I18N_NOOP("%{APPNAMELC}Part"), "0.0.1");
        aboutData->addAuthor("%{AUTHOR}", 0, "%{EMAIL}");
        return aboutData;
}

void %{APPNAMELC}Part::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_widget->setReadOnly(!rw);
    if (rw)
        connect(m_widget, SIGNAL(textChanged()),
                this,     SLOT(setModified()));
    else
    {
        disconnect(m_widget, SIGNAL(textChanged()),
                   this,     SLOT(setModified()));
    }

    ReadWritePart::setReadWrite(rw);
}

void %{APPNAMELC}Part::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

bool %{APPNAMELC}Part::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(QIODevice::ReadOnly) == false)
        return false;

    // our example widget is text-based, so we use QTextStream instead
    // of a raw QDataStream
    QTextStream stream(&file);
    QString str;
    while (!stream.eof())
        str += stream.readLine() + "\n";

    file.close();

    // now that we have the entire file, display it
    m_widget->setText(str);

    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

bool %{APPNAMELC}Part::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // m_file is always local, so we use QFile
    QFile file(m_file);
    if (file.open(QIODevice::WriteOnly) == false)
        return false;

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream << m_widget->text();

    file.close();

    return true;
}

void %{APPNAMELC}Part::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

void %{APPNAMELC}Part::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName();
    if (file_name.isEmpty() == false)
        saveAs(file_name);
}

#include "%{APPNAMELC}_part.moc"
