/*
 *  File : snippetsettings.h
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#ifndef SNIPPETSETTINGS_H
#define SNIPPETSETTINGS_H

#include "snippetsettingsbase.h"

class SnippetWidget;
class SnippetConfig;

/**
This class is the widget that is showen in the
KDevelop settings dialog. It inherits the
class SnippetSettingsBase which is created by the
same named .ui file
@author Robert Gruber
*/
class SnippetSettings : public SnippetSettingsBase
{
Q_OBJECT
public:
    SnippetSettings(QWidget *parent = 0, const char *name = 0);
    SnippetSettings(SnippetWidget * w, QWidget *parent = 0, const char *name = 0);

    ~SnippetSettings();

public slots:
    void slotOKClicked();

private:
  SnippetConfig * _cfg;
  SnippetWidget * _widget;
};

#endif
