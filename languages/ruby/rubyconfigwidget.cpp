

#include "rubyconfigwidget.h"
#include "domutil.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdebug.h>

RubyConfigWidget::RubyConfigWidget(QDomDocument &projectDom, QWidget* parent, const char* name)
: RubyConfigWidgetBase(parent,name), dom (projectDom) {
    kdDebug (9019) << "Creating RubyConfigWidget" << endl;
    interpreterEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/interpreter"));
    terminalCheckbox->setChecked(DomUtil::readBoolEntry(dom, "/kdevrubysupport/run/terminal"));
}

RubyConfigWidget::~RubyConfigWidget()
{
}

/*$SPECIALIZATION$*/
void RubyConfigWidget::accept() {
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/interpreter", interpreterEdit->text());
    DomUtil::writeBoolEntry(dom, "/kdevrubysupport/run/terminal", terminalCheckbox->isChecked());
}


#include "rubyconfigwidget.moc"

