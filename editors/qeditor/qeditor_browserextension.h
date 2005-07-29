/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef __qeditor_browserextension_h_
#define __qeditor_browserextension_h_

#include <kparts/browserextension.h>

class QEditorPart;

class QEditorBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    QEditorBrowserExtension( QEditorPart* doc );

public slots:
    void copy();
    void slotSelectionChanged();

private:
    QEditorPart* m_doc;
};

#endif // __qeditor_

