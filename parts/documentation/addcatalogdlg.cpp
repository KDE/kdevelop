/***************************************************************************
 *   Copyright (C) 2005 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
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

#include "addcatalogdlg.h"

#include <qlabel.h>
#include <qcombobox.h>

#include <klineedit.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kdebug.h>

#include "docutils.h"
#include "kdevdocumentationplugin.h"

AddCatalogDlg::AddCatalogDlg( QValueList<DocumentationPlugin*> const & plugins, 
	QWidget* parent, const char* name, bool modal, WFlags fl)
	:AddCatalogDlgBase(parent,name, modal,fl), m_plugins( plugins )
{
	for (QValueList<DocumentationPlugin*>::const_iterator it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it)
	{
		docTypeCombo->insertItem( (*it)->pluginName() );
	}
	
	docTypeChanged( QString() );
}

AddCatalogDlg::~AddCatalogDlg()
{
}

void AddCatalogDlg::locationURLChanged(const QString &text)
{
	DocumentationPlugin * plugin = this->plugin();
	if ( plugin ) return;
	
	titleEdit->setText( plugin->catalogTitle(DocUtils::noEnvURL(text)) );
}

void AddCatalogDlg::accept()
{
    QDialog::accept();
}

QString AddCatalogDlg::title() const
{
    return titleEdit->text();
}

QString AddCatalogDlg::url() const
{
    return DocUtils::envURL(locationURL);
}

void AddCatalogDlg::setTitle(const QString &title)
{
    titleEdit->setText(title);
}

void AddCatalogDlg::setURL(const QString &url)
{
    locationURL->setURL(url);
}

DocumentationPlugin * AddCatalogDlg::plugin( )
{
	for (QValueList<DocumentationPlugin*>::const_iterator it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it)
	{
		if ( docTypeCombo->currentText() == (*it)->pluginName() )
		{
			return *it;
		}
	}
	return 0;
}

void AddCatalogDlg::docTypeChanged( const QString & )
{
	kdDebug() << k_funcinfo << endl;

	DocumentationPlugin * plugin = this->plugin();
	if ( plugin )
	{
		titleLabel->setEnabled( plugin->hasCapability(DocumentationPlugin::CustomDocumentationTitles) );
		titleEdit->setEnabled( plugin->hasCapability(DocumentationPlugin::CustomDocumentationTitles) );
		locationURL->setMode( plugin->catalogLocatorProps().first );
		locationURL->setFilter( plugin->catalogLocatorProps().second );
	}
	
}



#include "addcatalogdlg.moc"
