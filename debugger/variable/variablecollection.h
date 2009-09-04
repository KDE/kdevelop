/*
 * KDevelop Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEV_VARIABLECOLLECTION_H
#define KDEV_VARIABLECOLLECTION_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <KDE/KTextEditor/Document>
#include <KDE/KTextEditor/View>

#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "../../interfaces/idocument.h"
#include "../debuggerexport.h"
#include "../interfaces/idebugsession.h"
#include "../../interfaces/idebugcontroller.h"

namespace GDBDebugger {
    class GdbTest;
}

namespace KDevelop
{
class VariableToolTip;

class KDEVPLATFORMDEBUGGER_EXPORT Variable : public TreeItem
{
    friend class GDBDebugger::GdbTest;
public:
protected:
    Variable(TreeModel* model, TreeItem* parent,
             const QString& expression,
             const QString& display = "");

public:

    QString varobj() const;
    QString expression() const;
    bool inScope() const;
    void setInScope(bool v);
    void setVarobj(const QString& v);
    void setValue(const QString &v);
    void setTopLevel(bool v);
    using TreeItem::setHasMore;
    using TreeItem::setHasMoreInitial;
    using TreeItem::appendChild;
    using TreeItem::deleteChildren;
    using TreeItem::isExpanded;

    using TreeItem::model;

    ~Variable();

    void createVarobjMaybe(QObject *callback = 0, const char *callbackMethod = 0);

    void die();

    void fetchMoreChildren();

    static Variable *findByName(const QString& name);
    /* Called when GDB dies.  Clears the association between varobj names
       and Variable instances.  */
    static void markAllDead();


private: // TreeItem overrides

    QVariant data(int column, int role) const;

private:

    QString expression_;
    QString varobj_;
    bool inScope_;
    bool topLevel_;

    static QMap<QString, Variable*> allVariables_;
};

class KDEVPLATFORMDEBUGGER_EXPORT TooltipRoot : public TreeItem
{
public:
    TooltipRoot(TreeModel* model)
    : TreeItem(model)
    {}

    void init(Variable *var)
    {
        appendChild(var);
    }

    void fetchMoreChildren() {}
};

class KDEVPLATFORMDEBUGGER_EXPORT Watches : public TreeItem
{
    friend class GDBDebugger::GdbTest;
public:
    Watches(TreeModel* model, TreeItem* parent);
    Variable* add(const QString& expression);

    void reinstall();

    Variable *addFinishResult(const QString& convenienceVarible);
    void removeFinishResult();

    using TreeItem::childCount;
    friend class VariableCollection;
private:

    QVariant data(int column, int role) const;

    void fetchMoreChildren() {}

    Variable* finishResult_;
};

class KDEVPLATFORMDEBUGGER_EXPORT Locals : public TreeItem
{
public:
    Locals(TreeModel* model, TreeItem* parent);
    void updateLocals(QStringList locals);

    using TreeItem::deleteChildren;
    using TreeItem::setHasMore;

    friend class VariableCollection;

private:
    void fetchMoreChildren() {}
};

class KDEVPLATFORMDEBUGGER_EXPORT VariablesRoot : public TreeItem
{
public:
    VariablesRoot(TreeModel* model);

    Watches *watches() const { return watches_; }
    Locals *locals() const { return locals_; }

    void fetchMoreChildren() {}

private:
    Watches *watches_;
    Locals *locals_;
};

class KDEVPLATFORMDEBUGGER_EXPORT VariableCollection : public TreeModel
{
    Q_OBJECT

public:
    VariableCollection(IDebugController* parent);
    virtual ~VariableCollection();

    VariablesRoot* root() const { return universe_; }
    Watches* watches() const { return universe_->watches(); }
    Locals* locals() const { return universe_->locals(); }

public Q_SLOTS:
    void variableWidgetShown();
    void variableWidgetHidden();

private Q_SLOTS:
    void updateAutoUpdate(KDevelop::IDebugSession* session = 0);

private Q_SLOTS:
    void textDocumentCreated( KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);
    void textHintRequested(const KTextEditor::Cursor&, QString&);

private:
    VariablesRoot* universe_;
    QPointer<VariableToolTip> activeTooltip_;
    bool m_widgetVisible;
};

}

#endif
