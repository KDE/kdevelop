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
 
#ifndef ADDCATALOGDLG_H
#define ADDCATALOGDLG_H

#include <qvaluelist.h>
#include "addcatalogdlgbase.h"

class DocumentationPlugin;

class AddCatalogDlg: public AddCatalogDlgBase
{
	Q_OBJECT
	public:
		AddCatalogDlg( QValueList<DocumentationPlugin*> const & plugins, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
		~AddCatalogDlg();
	
		QString title() const;
		void setTitle(const QString &title);
		QString url() const;
		void setURL(const QString &url);
		DocumentationPlugin * plugin();
		
	public slots:
		virtual void locationURLChanged(const QString &text);
		virtual void docTypeChanged( const QString & );
	
	protected slots:
		virtual void accept();
	
	private:
		QValueList<DocumentationPlugin*> const & m_plugins;
};

#endif
