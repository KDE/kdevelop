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
#include <kmdidefines.h>
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

  int mdi = config->readNumEntry("MDIMode", KMdi::IDEAlMode);
  
  switch (mdi)
  {
  case KMdi::ChildframeMode:
    modeMDI->setChecked(true);
    _lastMode = modeMDI;
    break;
  case KMdi::TabPageMode:
    modeTab->setChecked(true);
    _lastMode = modeTab;
    break;
  case KMdi::ToplevelMode:
    modeToplevel->setChecked(true);
    _lastMode = modeToplevel;
    break;
  case KMdi::IDEAlMode:
    modeIDEAl->setChecked(true);
    _lastMode = modeIDEAl;
    break;
  default:
    break;
  }
  
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
	
	int tabVisibility = config->readNumEntry( "TabWidgetVisibility", KMdi::AlwaysShowTabs );
	switch( tabVisibility )
	{
		case KMdi::AlwaysShowTabs:
			AlwaysShowTabs->setChecked( true );
			break;
		case KMdi::ShowWhenMoreThanOneTab:
			ShowWhenMoreThanOneTab->setChecked( true );
			break;
		case KMdi::NeverShowTabs:
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
}


void UIChooserWidget::save()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  if (modeTab->isChecked())
    config->writeEntry("MDIMode", KMdi::TabPageMode);
  else if (modeToplevel->isChecked())
    config->writeEntry("MDIMode", KMdi::ToplevelMode);
  else if (modeMDI->isChecked())
    config->writeEntry("MDIMode", KMdi::ChildframeMode);
  else
    config->writeEntry("MDIMode", KMdi::IDEAlMode); // KMdi-IDEA

	if ( AlwaysShowTabs->isChecked() )
	{
		config->writeEntry( "TabWidgetVisibility", KMdi::AlwaysShowTabs );
	}
	else if ( ShowWhenMoreThanOneTab->isChecked() )
	{
		config->writeEntry( "TabWidgetVisibility", KMdi::ShowWhenMoreThanOneTab );
	}
	else if ( NeverShowTabs->isChecked() )
	{
		config->writeEntry( "TabWidgetVisibility", KMdi::NeverShowTabs );
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
  
  // Note: with newmainwindow.cpp, these calls will be ignored
  
  if (modeIDEAl->isChecked() && _lastMode != modeIDEAl ) {
      m_part->mainWindow()->setUserInterfaceMode("KMDI-IDEAl");
  }
  else if (modeTab->isChecked() && _lastMode != modeTab ) {
      m_part->mainWindow()->setUserInterfaceMode("TabPage");
  }
  else if (modeToplevel->isChecked() && _lastMode != modeToplevel ) {
      m_part->mainWindow()->setUserInterfaceMode("Toplevel");
  }
  else if (modeMDI->isChecked() && _lastMode != modeMDI ) {
      m_part->mainWindow()->setUserInterfaceMode("Childframe");
  }
}

void UIChooserWidget::maybeEnableCloseOnHover( bool )
{
	if ( sender() == modeMDI || sender() == modeToplevel )
	{
		HoverCloseGroup->setEnabled( false );
	}
	else if ( !NeverShowTabs->isChecked() )
	{
		HoverCloseGroup->setEnabled( true );
	}
}


#include "uichooser_widget.moc"
