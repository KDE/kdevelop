/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VARIABLEWIDGET_H_
#define _VARIABLEWIDGET_H_

#include "gdbcontroller.h"
#include "mi/gdbmi.h"

#include <klistview.h>
#include <kcombobox.h>
#include <qwidget.h>
#include <qtooltip.h>
#include <qvaluevector.h>
#include <qdatetime.h>
#include <qguardedptr.h>
#include <qmap.h>

#include <vector>

class KLineEdit;
class KPopupMenu;

namespace GDBDebugger
{

class TrimmableItem;
class VarFrameRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;
class GDBBreakpointWidget;

enum { VarNameCol = 0, ValueCol = 1, VarTypeCol = 2};
enum DataType { typeUnknown, typeValue, typePointer, typeReference,
                typeStruct, typeArray, typeQString, typeWhitespace,
                typeName };

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( GDBController* controller,
                    GDBBreakpointWidget* breakpointWidget,
                    QWidget *parent=0, const char *name=0 );

    VariableTree *varTree() const
    { return varTree_; }

protected: // QWidget overrides
    void focusInEvent(QFocusEvent *e);

public slots:
    void slotAddWatchVariable();
    void slotAddWatchVariable(const QString &ident);
    void slotEvaluateExpression();
    void slotEvaluateExpression(const QString &ident);

private:
    VariableTree *varTree_;
//    KLineEdit *watchVarEntry_;
    friend class VariableTree;

    KHistoryCombo *watchVarEditor_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KListView, public QToolTip
{
    Q_OBJECT
public:
    VariableTree(VariableWidget *parent, 
                 GDBController*  controller,
                 GDBBreakpointWidget* breakpointWidget,
                 const char *name=0 );
    virtual ~VariableTree();

    QListViewItem *lastChild() const;

    QListViewItem *findRoot(QListViewItem *item) const;
    VarFrameRoot *findFrame(int frameNo, int threadNo) const;
    WatchRoot *findWatch();


	// (from QToolTip) Display a tooltip when the cursor is over an item
	virtual void maybeTip(const QPoint &);

    GDBController* controller() const { return controller_; }

signals:
    void toggleWatchpoint(const QString &varName);

public slots:
    void slotAddWatchVariable(const QString& watchVar);
    void slotEvaluateExpression(const QString& expression);

    void slotEvent(GDBController::event_t);
    void slotItemRenamed(QListViewItem* item, int col, const QString& text);

private slots:
    void slotContextMenu(KListView *, QListViewItem *item);
    void slotVarobjNameChanged(const QString& from, const QString& to);

private: // Callbacks for gdb commands;
    void argumentsReady(const GDBMI::ResultRecord&);
    void localsReady(const GDBMI::ResultRecord&);
    void frameIdReady(const QValueVector<QString>&);
    void handleVarUpdate(const GDBMI::ResultRecord&);
    void handleEvaluateExpression(const QValueVector<QString>&);
    void variablesFetchDone();
    void fetchSpecialValuesDone();
    
    /** This is called when address of expression for which 
        popup is created is known.

        If there's no address (for rvalue), does nothing
        (leaving "Data breakpoint" item disabled).
        Otherwise, enabled that item, and check is we 
        have data breakpoint for that address already.
    */
    void handleAddressComputed(const GDBMI::ResultRecord& r);

private: // helper functions
    /** Get (if exists) and create (otherwise) frame root for
        the specified frameNo/threadNo combination.
    */    
    VarFrameRoot* demand_frame_root(int frameNo, int threadNo);

    void updateCurrentFrame();

    /** Copies the value (second column) of the specified item to
        the clipboard.
    */
    void copyToClipboard(QListViewItem*);

private: // QWidget overrides
    void keyPressEvent(QKeyEvent* e);

private:
    GDBController*  controller_;
    GDBBreakpointWidget* breakpointWidget_;

    int activeFlag_;
    int iOutRadix;
    bool justPaused_;

    // Root of all recently printed expressions.
    TrimmableItem* recentExpressions_;
    VarFrameRoot* currentFrameItem;

    QTime fetch_time;
    // Names of locals and arguments as reported by
    // gdb.
    std::vector<QString> locals_and_arguments;

    QMap<QString, VarItem*> varobj2varitem;

    KPopupMenu* activePopup_;
    static const int idToggleWatch = 10;

    friend class VarFrameRoot;
    friend class VarItem;
    friend class WatchRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/** List view item that can 'trim' outdated children.

    The instances of this class hold a number of children corresponding
    to variables. When program state changes, such as after a step in source,
    some variable values can change, and some variables can go out of scope.
    We need
    - highlight modified variables
    - remove gone variables

    We could just remove all children and repopulate the list from
    the data from debugger, but then we'd loose information about previous
    variable values.

    So, we first update the values, highlighting the modified variables, and
    keeping track which variables were recieved from gdb. After that, the
    'trim' method is called, removing all variables which were not recieved
    from gdbr.    
 */
class TrimmableItem : public KListViewItem
{
public:
    TrimmableItem(VariableTree *parent);
    TrimmableItem(TrimmableItem *parent);

    virtual ~TrimmableItem();

    QListViewItem *lastChild() const;

protected:

    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarItem : public QObject,
                public TrimmableItem
{
    Q_OBJECT
public:
    enum format_t { natural, hexadecimal, decimal, character, binary };   

    /** Creates top-level variable item from the specified expression.
        Optionally, alternative display name can be provided.
    */
    VarItem( TrimmableItem *parent, 
             const QString& expression, 
             bool frozen = false);

    VarItem( TrimmableItem *parent, const GDBMI::Value& varobj,
             format_t format, bool baseClassMember);

    virtual ~VarItem();
    
    /// Returns the gdb expression for *this.
    QString gdbExpression() const;     

    /** Returns true is this VarItem should be unconditionally
        updated on each step, not matter what's the result of
        -var-update command.
    */
    bool updateUnconditionally() const;

    void updateValue();
    void updateSpecialRepresentation(const QString& s);

    /** Creates a fresh gdbs "variable object", if needed.
        Preconditions:
          - frame id did not change
          - this is a root variable

        If the current type of expression, or it's address, it different
        from it was previously, creates new "variable object" and
        fetches new value.

        Otherwise, does nothing.
    */
    void recreateLocallyMaybe();

    /** Tries to create new gdb variable object for this expression.
        If successfull, updates all values. Otherwise, makes
        itself disabled.
    */
    void recreate();

    void setOpen(bool open);
    void setText (int column, const QString& text);

    /** Mark the variable as alive, or not alive.
        Variables that are not alive a shown as "gray",
        and nothing can be done about them except for
        removing. */
    void setAliveRecursively(bool enable);

    /** Recursively clears the varobjName_ field, making
       *this completely disconnected from gdb.
       Automatically makes *this and children disables,
       since there's no possible interaction with unhooked
       object.
    */
    void unhookFromGdb();

    // Returns the text to be displayed as tooltip (the value)
    QString tipText() const;

    format_t format() const;
    void setFormat(format_t f);
    format_t formatFromGdbModifier(char c) const;

    /** Clears highliting for this variable and 
        all its children. */
    void clearHighlight();

    /** Sets new top-level textual value of this variable.
    */
    void setValue(const QString& new_value);

    bool isAlive() const;

signals:
    /** Emitted whenever the name of varobj associated with *this changes:
        - when we've created initial varobj
        - when we've changed varobj name as part of 'recreate' method
        - when *this is destroyed and no longer uses any varobj.

        Either 'from' or 'to' can be empty string.
    */
    void varobjNameChange(const QString& from, const QString& to);

private:

    /** Creates new gdb "variable object". The controller_,
        expression_ and format_ member variables should already
        be set.
     */
    void createVarobj();

    /** Precondition: 'name' is a name of existing
        gdb variable object.
        Effects: 
           - sets varobjName_ to 'name'
           - sets format, if it's not default one
           - gets initial value
           - if item is open, gets children.
    */
    void setVarobjName(const QString& name);


    /** Handle types that require special dispay, such as
        QString. Return true if this is such a type.
        The 'originalValueType_' is already initialized
        by the time this method is called.
    */
    bool handleSpecialTypes();
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );
    void varobjCreated(const GDBMI::ResultRecord& r);
    void valueDone(const GDBMI::ResultRecord& r);
    void childrenDone(const GDBMI::ResultRecord& r);    
    void childrenOfFakesDone(const GDBMI::ResultRecord& r);    
    void handleCurrentAddress(const QValueVector<QString>& lines);
    void handleType(const QValueVector<QString>& lines);

    void createChildren(const GDBMI::ResultRecord& r, bool children_of_fake);

    /** Called to handle the output of the cli print command.
     */
    void handleCliPrint(const QValueVector<QString>& lines);

    // Assuming 'expression_' is already set, returns the
    // displayName to use when showing this to the user.
    // This function exists because if we have item with
    // gdb expression '$1' and displayName 'P4', we want the child
    // to show up as *P4, not as '*$1', so we can't uncondionally
    // use expression gdb reports to us.
    QString displayName() const;

    VariableTree* varTree() const;

    QString varobjFormatName() const;

private:
    // The gdb expression for this varItem relatively to
    // parent VarItem. 
    QString expression_;

    bool      highlight_;
    GDBController* controller_;

    QString varobjName_;

    // the non-cast type of the variable    
    QString originalValueType_;
    bool oldSpecialRepresentationSet_;
    QString oldSpecialRepresentation_;

    format_t format_;

    static int varobjIndex;

    int numChildren_;
    bool childrenFetched_;
    
    QString currentAddress_;
    QString lastObtainedAddress_;

    bool updateUnconditionally_;
    bool frozen_;

    /* Set to true whan calling createVarobj for the
       first time, and to false other time. */
    bool initialCreation_;

    /* Set if this VarItem corresponds to base class suboject.  */
    bool baseClassMember_;

    bool alive_;
};



/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarFrameRoot : public TrimmableItem
{
public:
    VarFrameRoot(VariableTree *parent, int frameNo, int threadNo);
    virtual ~VarFrameRoot();

    void setOpen(bool open);

    // Marks the frame as dirty, that is as having
    // out of date values. As soon as we try to open
    // this item, it will fetch new data.
    void setDirty();

    void setFrameName(const QString &frameName)
                { setText(VarNameCol, frameName); setText(ValueCol, ""); }

    bool needLocals() const                     { return needLocals_; }
    bool matchDetails(int frameNo, int threadNo);

private:
    bool    needLocals_;
    int     frameNo_;
    int     threadNo_;

    // Frame base and code address of the current inner-most
    // frame. Needed so that if we can know when 'frame N' no longer
    // is the same as 'frame N' when this 'VarFrameRoot' was created.
    unsigned long long currentFrameBase;
    unsigned long long currentFrameCodeAddress;

    friend class VariableTree;

};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class WatchRoot : public TrimmableItem
{
public:
    WatchRoot(VariableTree *parent);
    virtual ~WatchRoot();
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
