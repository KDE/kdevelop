

#include "rubyconfigwidget.h"
#include "domutil.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <kdebug.h>

RubyConfigWidget::RubyConfigWidget(QDomDocument &projectDom, QWidget* parent, const char* name)
: RubyConfigWidgetBase(parent,name), dom (projectDom) {
    kdDebug (9019) << "Creating RubyConfigWidget" << endl;
    interpreterEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/interpreter"));
    programArgsEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/programargs"));
    terminalCheckbox->setChecked(DomUtil::readBoolEntry(dom, "/kdevrubysupport/run/terminal"));
    characterCodingRadioBox->setButton(DomUtil::readIntEntry(dom, "/kdevrubysupport/run/charactercoding"));
    enableFloatingToolBarBox->setChecked( DomUtil::readBoolEntry(dom, "/kdevrbdebugger/general/floatingtoolbar", false));
}

RubyConfigWidget::~RubyConfigWidget()
{
}

/*$SPECIALIZATION$*/
void RubyConfigWidget::accept() {
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/interpreter", interpreterEdit->text());
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/programargs", programArgsEdit->text());
    DomUtil::writeBoolEntry(dom, "/kdevrubysupport/run/terminal", terminalCheckbox->isChecked());
    DomUtil::writeIntEntry(dom, "/kdevrubysupport/run/charactercoding", characterCodingRadioBox->selectedId());
    DomUtil::writeBoolEntry(dom, "/kdevrbdebugger/general/floatingtoolbar", enableFloatingToolBarBox->isChecked());
}


#include "rubyconfigwidget.moc"

