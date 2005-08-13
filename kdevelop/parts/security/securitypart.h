/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#ifndef SECURITYPART_H
#define SECURITYPART_H

#include <kdevplugin.h>

class SecurityWidget;
class SecurityChecker;

namespace KParts {
    class Part;
}

namespace KTextEditor {
    class ViewCursorInterface;
    class EditInterface;
}

class SecurityPart: public KDevPlugin {
Q_OBJECT
public:
    SecurityPart(QObject *parent, const char *name, const QStringList &args);
    ~SecurityPart();

    SecurityWidget *widget() const { return m_widget; }
    QString activeFile() const { return m_activeFileName; }
    KTextEditor::ViewCursorInterface *activeViewCursor() const { return m_activeViewCursor; }
    KTextEditor::EditInterface *activeEditor() const { return m_activeEditInterface; }

public slots:
    void activePartChanged(KParts::Part *part);    
    
private:
    SecurityWidget *m_widget;
    SecurityChecker *m_checker;
    KParts::Part *m_activeEditor;
    KTextEditor::EditInterface *m_activeEditInterface;
    KTextEditor::ViewCursorInterface *m_activeViewCursor;

    QString m_activeFileName;
};

#endif
