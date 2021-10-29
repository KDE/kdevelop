#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

class Member(object):
    def __init__(self, name):
        """
            Member of classes are identified by a name. A parent name can be
            given and will be used to build the string "Parent.prototype.name"
        """
        self._name = name
        self._full_name = name

    def setParentName(self, name, usePrototype = True):
        if usePrototype:
            self._full_name = '%s.prototype.%s' % (name, self.name())
        else:
            self._full_name = '%s.%s' % (name, self.name())

    def setName(self, name):
        self._name = name

    def name(self):
        return self._name

    def fullName(self):
        return self._full_name

    def print(self):
        print('%s = %s;' % (self.fullName(), self.valueToAssign()))

        if self.valueAfterAssignation() != '':
            print(self.valueAfterAssignation())

    def valueToAssign(self):
        """
            Value to assign to the member, for instance "function (){}"
        """
        return '""';

    def valueAfterAssignation(self):
        """
            Line that will be printed after the assignation. Can be used to call
            a function (therefore giving the type of its parameters.
        """
        return '';

class F(Member):
    def __init__(self, returnValue, name, *args):
        """
            A function has a name, a return value, and arguments. Each argument,
            is a tuple whose first entry is the argument name, and the second
            one its type.

            Note that types are not given using names ("int", "bool", etc), but
            values of the type ("1" for an int, "true" or "false" for a boolean,
            "new X()" for class X, etc).
        """
        Member.__init__(self, name)

        self._return_value = returnValue
        self._args = args

    def print(self):
        if self.name() != '':
            # This function is not a member, no need to assign it to an object
            print(self.valueToAssign())
        else:
            Member.print(self)

    def valueToAssign(self):
        # Define the function
        return 'function %s(%s) { return %s; }' % (
            self.name(),
            ', '.join([arg[0] for arg in self._args]),
            self._return_value
        )

    def valueAfterAssignation(self):
        # Call it, so that its parameters have the correct type
        return '%s(%s);' % (
            self.fullName(),
            ', '.join([arg[1] for arg in self._args])
        )

class Var(Member):
    def __init__(self, type, name):
        """
            A variable has a name and a type
        """
        Member.__init__(self, name)

        self._type = type

    def print(self):
        if self.name() != '':
            # This variable is not a member, declare it using 'var'
            print('var %s = %s;' % (self.name(), self.valueToAssign()))
        else:
            Member.print(self)

    def valueToAssign(self):
        return self._type


class Class(F):
    def __init__(self, name, *args):
        F.__init__(self, '', name, *args)
        self._members = []
        self._prototype = None

    def prototype(self, proto):
        self._prototype = proto
        return self

    def member(self, member):
        self._members.append(member)
        return self

    def members(self, *args):
        for arg in args:
            self.member(arg)

        return self

    def print(self):
        # Declare the constructor (a function)
        print('/*\n * %s\n */' % self.fullName())

        # Always declare a function using the function keyword
        old_full_name = self._full_name
        old_name = self._name

        self._full_name = self._full_name.split('.')[-1]
        self._name = self._full_name

        F.print(self)

        if old_full_name != self._name:
            print('%s = %s;' % (old_full_name, self._name))

        self._full_name = old_full_name
        self._name = old_name

        # Print the prototype
        if self._prototype is not None:
            print('%s.prototype = %s;' % (self.fullName(), self._prototype))

        print('')

        # Declare the members
        for member in self._members:
            member.setParentName(self.fullName())
            member.setName('')
            member.print()
            print('')

class Struct(Var):
    def __init__(self, name):
        Var.__init__(self, '{}', name)
        self._members = []

    def member(self, member):
        self._members.append(member)
        return self

    def members(self, *args):
        for arg in args:
            self.member(arg)

        return self

    def print(self):
        # Declare the object
        print('/*\n * %s\n */' % self.fullName())
        Var.print(self)
        print('')

        # Declare the members
        for member in self._members:
            member.setParentName(self.fullName(), False)
            member.setName('')
            member.print()
            print('')

class Module(object):
    def __init__(self):
        self._members = []

    def member(self, member):
        self._members.append(member)
        return self

    def members(self, *args):
        for arg in args:
            self.member(arg)

        return self

    def print(self):
        # Declare the members in "exports"
        for member in self._members:
            member.setParentName('exports', False)
            member.setName('')
            member.print()
            print('')
