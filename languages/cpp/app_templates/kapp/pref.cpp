//Added by qt3to4:
#include <Q3Frame>
#include <QHBoxLayout>
%{CPP_TEMPLATE}

#include "pref.h"

#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

%{APPNAME}Preferences::%{APPNAME}Preferences()
    : KDialogBase(TreeList, i18n("%{APPNAME} Preferences"),
                  Help|Default|Ok|Apply|Cancel, Ok)
{
    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    Q3Frame *frame;
    frame = addPage(i18n("First Page"), i18n("Page One Options"));
    m_pageOne = new %{APPNAME}PrefPageOne(frame);

    frame = addPage(i18n("Second Page"), i18n("Page Two Options"));
    m_pageTwo = new %{APPNAME}PrefPageTwo(frame);
}

%{APPNAME}PrefPageOne::%{APPNAME}PrefPageOne(QWidget *parent)
    : Q3Frame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel(i18n("Add something here"), this);
}

%{APPNAME}PrefPageTwo::%{APPNAME}PrefPageTwo(QWidget *parent)
    : Q3Frame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel(i18n("Add something here"), this);
}
#include "pref.moc"
