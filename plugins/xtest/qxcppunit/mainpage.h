/*!
 * \file  mainpage.h
 *
 * \brief Contains documentation.
 *
 * This file contains text and doxygen formatting commands for the
 * \ref main "Main Page" and other parts of the API documentation.
 */

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page license License
\verbatim

QxCppUnit Library
Copyright (C) 2006 systest.ch

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free 
Software Foundation; either version 2.1 of the License, or (at your option) 
any later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more 
details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, Inc., 
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

\endverbatim
*/


namespace QxCppUnit {	// Enables doxygen to create links to documented items


/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\mainpage

\image html qxcppunit_64x64.png

<!------------------------------------------------------------------->

\section intro Introduction

The QxCppUnit library provides a GUI frontend for CppUnit test execution.
It is based on the QxRunner and Qt libraries. It can be used to unit test
any kind of C++ software, it's not limited to Qt applications.

This library is just a thin wrapper around the QxRunner library.
Actually the idea to write a Qt test runner with a user-friendly GUI
led to the development of the QxRunner library.

The QxCppUnit library mainly consists of the CppUnitItem class, which
subclasses QxRunner::RunnerItem, and the CppUnitModel class derived from
QxRunner::RunnerModel. The TestRunner class populates a CppUnitModel
instance with CppUnitItem objects and uses a QxRunner::Runner instance
to show the GUI on the screen.

To write a GUI test runner for CppUnit just the TestRunner class
is needed as described in the \ref examples page.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page implementation_ascpects Implementation Aspects

<!------------------------------------------------------------------->

\section symbols Exported Symbols

This applies for the Win32 world only. By default symbols of the library
are 'hidden' when created as a DLL. Therefore symbols, in particular
classes, that must be accessible from the outside of the library are
exported with the QXCPPUNIT_EXPORT macro:

\code
class QXCPPUNIT_EXPORT TestRunner
{
public: // Operations
	...
\endcode

Only the TestRunner class is exported right now, the other classes are
for internal use only. This might be reconsidered in the future and more
classes could become exported to be used somewhere outside the library.

<!------------------------------------------------------------------->

\section result_types Result Types

See the CppUnitModel class for the result types handled by the library.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\namespace QxCppUnit

\brief Namespace for the QxCppUnit library entities.

\namespace QxRunner

\brief Namespace of the
<a target="_blank" href="http://qxrunner.systest.ch">QxRunner Library</a>.

\namespace CPPUNIT_NS

\brief The CppUnit library exposes its classes in the CPPUNIT_NS namespace.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page examples Examples

The demo program shipped with the QxCppUnit library can be used as the
starting point for writing a GUI test runner for CppUnit. It has some
CppUnit test fixtures with simple test cases which succeed or fail and
uses a TestRunner instance in the main program to launch the GUI.

\section demo_test_example A CppUnit Test Example

This is one of the test fixtures of the demo program. Of course this is a
very simple test and only used for demonstration purposes. It isn't
intended to show how to write CppUnit tests.

\includelineno testexamples2.cpp

\section demo_main The Demo Main Program

The main program is straightforward:

\includelineno main.cpp
*/

} // namespace

