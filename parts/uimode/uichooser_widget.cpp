/***************************************************************************
  uichooser_widget.cpp - ?
			     -------------------
    begin                : ?
    copyright            : (C) 2003 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>

#include "uichooser_part.h"
#include "kdevmainwindow.h"
#include "uichooser_widget.h"

UIChooserWidget::UIChooserWidget( UIChooserPart * part, QWidget *parent, const char *name)
  : UIChooser(parent, name)
  ,m_part(part), _lastMode(0L)
{
  load();
}

void UIChooserWidget::load()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

	int mdistyle = config->readNumEntry( "MDIStyle", 1 );
	switch( mdistyle )
	{
		case 0:
			IconsOnly->setChecked( true );
			break;
		case 1:
			TextOnly->setChecked( true );
			break;
		case 3:
			TextAndIcons->setChecked( true );
			break;
		default:
			TextOnly->setChecked( true );
	}

	int tabVisibility = config->readNumEntry( "TabWidgetVisibility", _AlwaysShowTabs );
	switch( tabVisibility )
	{
		case _AlwaysShowTabs:
			AlwaysShowTabs->setChecked( true );
			break;
		case _NeverShowTabs:
			NeverShowTabs->setChecked( true );
			break;
	}

	bool CloseOnHover = config->readBoolEntry( "CloseOnHover", false );
	bool CloseOnHoverDelay = config->readBoolEntry( "CloseOnHoverDelay", false );

	if ( CloseOnHover && CloseOnHoverDelay )
	{
		DoDelayedCloseOnHover->setChecked( true );
	}
	else if ( CloseOnHover && !CloseOnHoverDelay )
	{
		DoCloseOnHover->setChecked( true );
	}
	else
	{
		DoNotCloseOnHover->setChecked( true );
	}
	OpenNewTabAfterCurrent->setChecked(config->readBoolEntry( "OpenNewTabAfterCurrent", false ));
	ShowTabIcons->setChecked(config->readBoolEntry( "ShowTabIcons", true ));
	ShowCloseTabsButton->setChecked(config->readBoolEntry( "ShowCloseTabsButton", true ));

	maybeEnableCloseOnHover(false);
}


void UIChooserWidget::save()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

	if ( AlwaysShowTabs->isChecked() )
	{
		config->writeEntry( "TabWidgetVisibility", _AlwaysShowTabs );
	}
	else if ( NeverShowTabs->isChecked() )
	{
		config->writeEntry( "TabWidgetVisibility", _NeverShowTabs );
	}

	if ( DoNotCloseOnHover->isChecked() )
	{
		config->writeEntry( "CloseOnHover", false );
		config->writeEntry( "CloseOnHoverDelay", false );
	}
	else if ( DoCloseOnHover->isChecked() )
	{
		config->writeEntry( "CloseOnHover", true );
		config->writeEntry( "CloseOnHoverDelay", false );
	}
	else if ( DoDelayedCloseOnHover->isChecked() )
	{
		config->writeEntry( "CloseOnHover", true );
		config->writeEntry( "CloseOnHoverDelay", true );
	}

	// using magic numbers for now.. where are these values defined??
	if ( IconsOnly->isChecked() )
	{
		config->writeEntry( "MDIStyle", 0 );
	}
	else if ( TextAndIcons->isChecked() )
	{
		config->writeEntry( "MDIStyle", 3 );
	}
	else // TextOnly
	{
		config->writeEntry( "MDIStyle", 1 );
	}
	config->writeEntry("OpenNewTabAfterCurrent", OpenNewTabAfterCurrent->isChecked());
	config->writeEntry("ShowTabIcons", ShowTabIcons->isChecked());
	config->writeEntry("ShowCloseTabsButton", ShowCloseTabsButton->isChecked());

	config->sync();
}


void UIChooserWidget::accept()
{
  save();
}

void UIChooserWidget::maybeEnableCloseOnHover( bool )
{
	if (!ShowTabIcons->isChecked())
	{
		HoverCloseGroup->setEnabled(false);
	} else if ( !NeverShowTabs->isChecked() )
	{
		HoverCloseGroup->setEnabled( true );
	}
}


#include "uichooser_widget.moc"
