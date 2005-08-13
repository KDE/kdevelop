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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef EDITCATALOGDLG_H
#define EDITCATALOGDLG_H

#include "editcatalogdlgbase.h"

class DocumentationPlugin;

class EditCatalogDlg : public EditCatalogBase
{
    Q_OBJECT
public:
    EditCatalogDlg(DocumentationPlugin *plugin, 
        QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~EditCatalogDlg();
    /*$PUBLIC_FUNCTIONS$*/

    QString title() const;
    void setTitle(const QString &title);
    QString url() const;
    void setURL(const QString &url);
    
public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void locationURLChanged(const QString &text);

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
    virtual void accept();

private:
    DocumentationPlugin *m_plugin;
};

#endif
