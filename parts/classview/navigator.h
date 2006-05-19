/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
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
#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <qobject.h>
#include <codemodel.h>
#include <qfont.h>

class TextPaintStyleStore;
class QTimer;
class ClassViewPart;
class QListViewItem;

#define NAV_NODEFINITION "(no function)"

class TextPaintItem;
TextPaintItem highlightFunctionName( QString function, int type, TextPaintStyleStore& styles );

class Navigator : public QObject
{
Q_OBJECT
public:
    enum NavigationState { GoToDefinitions, GoToDeclarations };

    Navigator(ClassViewPart *parent, const char *name = 0);
    ~Navigator();

    void stopTimer();
    void refreshNavBars(const QString &activeFileName, bool clear = true);

    QString fullFunctionDefinitionName(FunctionDefinitionDom fun);
    QString fullFunctionDeclarationName(FunctionDom fun);

    template <class DomType>
    TextPaintItem fullFunctionItem(DomType fun);
    
public slots:
    void selectFunctionNav(QListViewItem *item);
    void syncFunctionNav();
    void syncFunctionNavDelayed(int delay);
    void functionNavFocused();
    void functionNavUnFocused();
    void slotCursorPositionChanged();
    void refresh();
    void addFile(const QString &file);

protected:

    FunctionDom currentFunction();

private:
    ClassViewPart *m_part;
    QTimer *m_syncTimer;
    NavigationState m_state;

    bool m_navNoDefinition;

    QMap<QString, QListViewItem*> m_functionNavDefs;
    QMap<QString, QListViewItem*> m_functionNavDecls;
    
    TextPaintStyleStore m_styles;
};

#endif
