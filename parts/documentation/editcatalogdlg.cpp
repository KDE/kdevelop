/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "editcatalogdlg.h"

#include <qlabel.h>

#include <klineedit.h>
#include <kurlrequester.h>

#include "kdevdocumentationplugin.h"

EditCatalogDlg::EditCatalogDlg(DocumentationPlugin *plugin, QWidget* parent,
    const char* name, bool modal, WFlags fl)
    :EditCatalogBase(parent,name, modal,fl), m_plugin(plugin)
{
    if (m_plugin->hasCapability(DocumentationPlugin::CustomDocumentationTitles))
    {
        titleLabel->setEnabled(true);
        titleEdit->setEnabled(true);
    }
    locationURL->setMode(m_plugin->catalogLocatorProps().first);
    locationURL->setFilter(m_plugin->catalogLocatorProps().second);
}

EditCatalogDlg::~EditCatalogDlg()
{
}

/*$SPECIALIZATION$*/
void EditCatalogDlg::locationURLChanged(const QString &text)
{
    titleEdit->setText(m_plugin->catalogTitle(text));
}

void EditCatalogDlg::accept()
{
    QDialog::accept();
}

QString EditCatalogDlg::title() const
{
    return titleEdit->text();
}

QString EditCatalogDlg::url() const
{
    return locationURL->url();
}

void EditCatalogDlg::setTitle(const QString &title)
{
    titleEdit->setText(title);
}

void EditCatalogDlg::setURL(const QString &url)
{
    locationURL->setURL(url);
}

#include "editcatalogdlg.moc"
