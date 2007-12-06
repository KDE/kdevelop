/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef _VARIABLEITEM_H_
#define _VARIABLEITEM_H_

#include "mi/gdbmi.h"

namespace GDBDebugger
{

class GDBController;
class VariableCollection;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableItem : public QObject
{
    Q_OBJECT

public:
    enum Columns {
      ColumnName = 0,
      ColumnValue = 1,
      ColumnType = 2,
      ColumnLast = ColumnType
    };

    enum DataTypes { typeUnknown, typeValue, typePointer, typeReference,
                typeStruct, typeArray, typeQString, typeWhitespace,
                typeName };

    enum format_t { natural, hexadecimal, decimal, character, binary };

    /** Creates top-level variable item from the specified expression.
        Optionally, alternative display name can be provided.
    */
    VariableItem( VariableItem* parent,
             const QString& expression,
             bool frozen = false);

    VariableItem( VariableItem* parent, const GDBMI::Value& varobj,
             format_t format, bool baseClassMember);

    virtual ~VariableItem();

    Qt::ItemFlags flags(int column) const;
    QVariant data(int column, int role = Qt::DisplayRole ) const;

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

    void getChildren();

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

    VariableItem* parent() const;
    const QList<VariableItem*>& children() const;

    void setVariableName(const QString& name);

Q_SIGNALS:
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
    //bool handleSpecialTypes();
    void varobjCreated(const GDBMI::ResultRecord& r);
    void valueDone(const GDBMI::ResultRecord& r);
    void childrenDone(const GDBMI::ResultRecord& r);
    void childrenOfFakesDone(const GDBMI::ResultRecord& r);
    void handleCurrentAddress(const QStringList& lines);
    void handleType(const QStringList& lines);

    void createChildren(const GDBMI::ResultRecord& r, bool children_of_fake);

    /** Called to handle the output of the cli print command.
     */
    void handleCliPrint(const QStringList& lines);

    // Assuming 'expression_' is already set, returns the
    // displayName to use when showing this to the user.
    // This function exists because if we have item with
    // gdb expression '$1' and displayName 'P4', we want the child
    // to show up as *P4, not as '*$1', so we can't uncondionally
    // use expression gdb reports to us.
    QString displayName() const;

    QString varobjFormatName() const;

private:
    VariableCollection* m_collection;
  
    // The gdb expression for this varItem relatively to
    // parent VarItem.
    QString expression_;

    bool      highlight_;

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

    QList<VariableItem*> m_children;
    QString m_value, m_type;
};

}

#endif
