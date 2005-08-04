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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <qobject.h>
#include <codemodel.h>

class QTimer;
class ClassViewPart;
class Q3ListViewItem;

#define NAV_NODEFINITION "(no function)"

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

public slots:
    void selectFunctionNav(Q3ListViewItem *item);
    void syncFunctionNav();
    void syncFunctionNavDelayed(int delay);
    void functionNavFocused();
    void functionNavUnFocused();
    void slotCursorPositionChanged();
    void refresh();
    void addFile(const QString &file);

protected:
    FunctionDefinitionDom currentFunctionDefinition();
    FunctionDefinitionDom functionDefinitionAt(int line, int column);
    FunctionDefinitionDom functionDefinitionAt(NamespaceDom ns, int line, int column);
    FunctionDefinitionDom functionDefinitionAt(ClassDom klass, int line, int column);
    FunctionDefinitionDom functionDefinitionAt(FunctionDefinitionDom fun, int line, int column);

    FunctionDom currentFunctionDeclaration();
    FunctionDom functionDeclarationAt(int line, int column);
    FunctionDom functionDeclarationAt(NamespaceDom ns, int line, int column);
    FunctionDom functionDeclarationAt(ClassDom klass, int line, int column);
    FunctionDom functionDeclarationAt(FunctionDom fun, int line, int column);

private:
    ClassViewPart *m_part;
    QTimer *m_syncTimer;
    NavigationState m_state;

    bool m_navNoDefinition;

    QMap<QString, Q3ListViewItem*> m_functionNavDefs;
    QMap<QString, Q3ListViewItem*> m_functionNavDecls;
};

#endif
