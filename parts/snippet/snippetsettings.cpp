/*
 *  File : snippetsettings.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qstring.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>

#include "snippetsettings.h"
#include "snippet_widget.h"


SnippetSettings::SnippetSettings(QWidget *parent, const char *name)
 : SnippetSettingsBase(parent, name)
{
  _widget = NULL;
}

SnippetSettings::SnippetSettings(SnippetWidget * w, QWidget *parent, const char *name)
 : SnippetSettingsBase(parent, name)
{
  _cfg = w->getSnippetConfig();
  _widget = w;
}


SnippetSettings::~SnippetSettings()
{
}


/*!
    \fn SnippetSettings::slotOKClicked()
 */
void SnippetSettings::slotOKClicked()
{
    _cfg->setToolTips(cbToolTip->isChecked());
    _cfg->setDelimiter(leDelimiter->text());
    _cfg->setInputMethod(btnGroup->selectedId());
    _cfg->setAutoOpenGroups(btnGroupAutoOpen->selectedId());

    if (_widget)
        _widget->languageChanged();
}


#include "snippetsettings.moc"
