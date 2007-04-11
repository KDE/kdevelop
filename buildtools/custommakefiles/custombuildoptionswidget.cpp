/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "custombuildoptionswidget.h"

#include <qcheckbox.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kfiledialog.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include "domutil.h"


CustomBuildOptionsWidget::CustomBuildOptionsWidget(QDomDocument &dom,
                                                   QWidget *parent, const char *name)
    : CustomBuildOptionsWidgetBase(parent, name),
      m_dom(dom)
{
    ant_button->setChecked(DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant");
    other_button->setChecked(DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "other");
    if( !DomUtil::readEntry(dom, "/kdevcustomproject/build/builddir").isEmpty()
            && QFileInfo( DomUtil::readEntry(dom, "/kdevcustomproject/build/builddir") ).exists() )
    {
        builddir_edit->setURL(DomUtil::readEntry(dom, "/kdevcustomproject/build/builddir"));
        builddir_edit->fileDialog()->setURL( DomUtil::readEntry(dom, "/kdevcustomproject/build/builddir") );
    }
    else
    {
        builddir_edit->setURL( QString() );
        builddir_edit->fileDialog()->setURL( QString() );
    }
    builddir_edit->completionObject()->setMode(KURLCompletion::DirCompletion);
    builddir_edit->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    // This connection must not be made before the ant->setChecked() line,
    // because at this time makeToggled() would crash
    connect( make_button, SIGNAL(toggled(bool)),
            this, SLOT(makeToggled(bool)) );
    connect( other_button, SIGNAL(toggled(bool)),
             this, SLOT(otherToggled(bool)) );
}


CustomBuildOptionsWidget::~CustomBuildOptionsWidget()
{}


void CustomBuildOptionsWidget::accept()
{
    QString buildtool;
    if (ant_button->isChecked())
    {
        buildtool = "ant";
    }
    else if (other_button->isChecked())
    {
        buildtool = "other";
    }
    else
    {
        buildtool = "make";
    }
    DomUtil::writeEntry(m_dom, "/kdevcustomproject/build/buildtool", buildtool);
    DomUtil::writeEntry(m_dom, "/kdevcustomproject/build/builddir", builddir_edit->url());
}


void CustomBuildOptionsWidget::setMakeOptionsWidget(QTabWidget *tw, QWidget *mow, QWidget* oow)
{
    m_tabWidget = tw;
    m_makeOptions = mow;
    m_otherOptions = oow;
    makeToggled(make_button->isChecked());
    otherToggled(other_button->isChecked());
}

void CustomBuildOptionsWidget::otherToggled(bool b)
{
    m_tabWidget->setTabEnabled(m_otherOptions, b);
}

void CustomBuildOptionsWidget::makeToggled(bool b)
{
    m_tabWidget->setTabEnabled(m_makeOptions, b);
}

#include "custombuildoptionswidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
