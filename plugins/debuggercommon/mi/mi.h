/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2005-2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GDBMI_H
#define GDBMI_H

#include <QString>
#include <QMap>

#include <stdexcept>

class QDebug;

/**
@author Roberto Raggi
@author Vladimir Prus
*/
namespace KDevMI { namespace MI {
    enum CommandType {
        NonMI,

        BreakAfter,
        BreakCommands,
        BreakCondition,
        BreakDelete,
        BreakDisable,
        BreakEnable,
        BreakInfo,
        BreakInsert,
        BreakList,
        BreakWatch,

        DataDisassemble,
        DataEvaluateExpression,
        DataListChangedRegisters,
        DataListRegisterNames,
        DataListRegisterValues,
        DataReadMemory,
        DataWriteMemory,
        DataWriteRegisterVariables,

        EnablePrettyPrinting,
        EnableTimings,

        EnvironmentCd,
        EnvironmentDirectory,
        EnvironmentPath,
        EnvironmentPwd,

        ExecAbort,
        ExecArguments,
        ExecContinue,
        ExecFinish,
        ExecInterrupt,
        ExecNext,
        ExecNextInstruction,
        ExecRun,
        ExecStep,
        ExecStepInstruction,
        ExecUntil,

        FileExecAndSymbols,
        FileExecFile,
        FileListExecSourceFile,
        FileListExecSourceFiles,
        FileSymbolFile,

        GdbExit,
        GdbSet,
        GdbShow,
        GdbVersion,

        InferiorTtySet,
        InferiorTtyShow,

        InterpreterExec,

        ListFeatures,

        SignalHandle,

        StackInfoDepth,
        StackInfoFrame,
        StackListArguments,
        StackListFrames,
        StackListLocals,
        StackSelectFrame,

        SymbolListLines,

        TargetAttach,
        TargetDetach,
        TargetDisconnect,
        TargetDownload,
        TargetSelect,

        ThreadInfo,
        ThreadListIds,
        ThreadSelect,

        TraceFind,
        TraceStart,
        TraceStop,

        VarAssign,
        VarCreate,
        VarDelete,
        VarEvaluateExpression,
        VarInfoPathExpression,
        VarInfoNumChildren,
        VarInfoType,
        VarListChildren,
        VarSetFormat,
        VarSetFrozen,
        VarShowAttributes,
        VarShowFormat,
        VarUpdate
    };

    /** Exception that is thrown when we're trying to invoke an
        operation that is not supported by specific MI value. For
        example, trying to index a string literal.

        Such errors are conceptually the same as assert, but in GUI
        we can't use regular assert, and Q_ASSERT, which only prints
        a message, is not suitable either. We need to break processing,
        and the higher-level code can report "Internal parsing error",
        or something.

        Being glorified assert, this exception does not cary any
        useful information.
    */
    class type_error : public std::logic_error
    {
    public:
        type_error();
    };

    /** Base class for all MI values.
        MI values are of three kinds:
        - String literals
        - Lists (indexed by integer)
        - Tuple (set of named values, indexed by name)

        The structure of response to a specific gdb command is fixed.
        While any tuples in response may omit certain named fields, the
        kind of each item never changes. That is, response to specific
        command can't contains sometimes string and sometimes tuple in
        specific position.

        Because of that static structure, it's almost never needed to query
        dynamic type of a MI value. Most often we know it's say, tuple, and
        can subscripts it. 

        So, the Value class has methods for accessing all kinds of values.
        Attempting to call a method that is not applicable to specific value
        will result in exception. The client code will almost never need to
        cast from 'Value' to its derived classes.

        Note also that all methods in this class are const and return 
        const Value&. That's by design -- there's no need to modify gdb
        responses in GUI.
     */
    struct Value
    {
        enum Kind {
            StringLiteral,
            Tuple,
            List
        };

    protected:
        constexpr explicit Value(Kind k) : kind(k) {}

    public:
        virtual ~Value() = default;

        Value() = delete;
        // Copy disabled to prevent slicing.
        Value(const Value&) = delete;
        Value& operator=(const Value&) = delete;

        const Kind kind = StringLiteral;

        /** If this value is a string literals, returns the string value.
            Otherwise, throws type_error.
        */
        virtual QString literal() const;

        //NOTE: Wouldn't it be better to use literal().toInt and get rid of that?
        /** If the value is a string literal, converts it to int and
            returns. If conversion fails, or the value cannot be
            converted to int, throws type_error.
        */
        virtual int toInt(int base = 10) const;

        /** If this value is a tuple, returns true if the tuple
            has a field named 'variable'. Otherwise,
            throws type_error.
        */
        virtual bool hasField(const QString& variable) const;

        /** If this value is a tuple, and contains named field 'variable',
            returns it. Otherwise, throws 'type_error'.
            This method is virtual, and derived in base class, so that
            we can save on casting, when we know for sure that instance
            is TupleValue, or ListValue.
        */
        virtual const Value& operator[](const QString& variable) const;

        /** If this value is a list, returns true if the list is empty.
            If this value is not a list, throws 'type_error'.
        */
        virtual bool empty() const;

        /** If this value is a list, returns it's size.
            Otherwise, throws 'type_error'.
        */
        virtual int size() const;

        /** If this value is a list, returns the element at
            'index'. Otherwise, throws 'type_error'.
        */
        virtual const Value& operator[](int index) const;

        /**
         * Write this value to a given stream.
         */
        virtual void print(QDebug debug) const = 0;
    };

    /** @internal
        Internal class to represent name-value pair in tuples.
    */
    struct Result
    {
        Result() = default;
        ~Result() { delete value; value = nullptr; }

        QString variable;
        Value *value = nullptr;

    private:
        Q_DISABLE_COPY(Result)
    };

    struct StringLiteralValue : public Value
    {
        explicit StringLiteralValue(const QString &lit)
            : Value(StringLiteral)
            , literal_(lit)
        {}

    public: // Value overrides

        QString literal() const override;
        int toInt(int base) const override;

        void print(QDebug debug) const override;

    private:
        QString literal_;
    };

    struct TupleValue : public Value
    {
        TupleValue() : Value(Tuple) {}
        ~TupleValue() override;

        bool hasField(const QString&) const override;

        using Value::operator[];
        const Value& operator[](const QString& variable) const override;

        void print(QDebug debug) const override;

        QList<Result*> results;
        QMap<QString, Result*> results_by_name;
    };

    struct ListValue : public Value
    {
        ListValue() : Value(List) {}
        ~ListValue() override;

        bool empty() const override;

        int size() const override;

        using Value::operator[];
        const Value& operator[](int index) const override;

        void print(QDebug debug) const override;

        QList<Result*> results;
    };

    QDebug operator<<(QDebug debug, const Value&);
    QDebug operator<<(QDebug debug, const Result*);

    struct Record
    {
        enum Kind {
            Prompt,
            Stream,
            Result,
            Async
        };

    protected:
        constexpr explicit Record(Kind k) : kind(k) {}

    public:
        Record() = delete;
        Record(const Record&) = delete;
        Record& operator=(const Record&) = delete;

        virtual ~Record() = default;
        virtual QString toString() const { Q_ASSERT( 0 ); return QString(); }

        const Kind kind;
    };

    struct TupleRecord : public Record, public TupleValue
    {
    protected:
        explicit TupleRecord(Record::Kind k) : Record(k) {}
    };

    struct ResultRecord : public TupleRecord
    {
        explicit ResultRecord(const QString& reason)
            : TupleRecord(Result)
            , reason(reason)
        {
        }

        uint32_t token = 0;
        QString reason;
    };

    struct AsyncRecord : public TupleRecord
    {
        enum Subkind {
            Exec,
            Status,
            Notify
        };

        AsyncRecord(Subkind subkind, const QString& reason)
            : TupleRecord(Async)
            , subkind(subkind)
            , reason(reason)
        {
        }

        Subkind subkind;
        QString reason;
    };

    struct PromptRecord : public Record
    {
        PromptRecord() : Record(Prompt) {}

        QString toString() const override
        { return QStringLiteral("(prompt)\n"); }
    };

    struct StreamRecord : public Record
    {
        enum Subkind {
            /// Console stream: usual CLI output of GDB in response to non-MI commands
            Console,

            /// Target output stream (stdout/stderr of the inferior process, only in some
            /// scenarios - usually we get stdout/stderr via other means)
            Target,

            /// Log stream: GDB internal messages that should be displayed as part of an error log
            Log
        };

        explicit StreamRecord(Subkind subkind)
            : Record(Stream)
            , subkind(subkind)
        {
        }

        Subkind subkind;
        QString message;
    };
} // end of namespace MI
} // end of namespace KDevMI

#endif
