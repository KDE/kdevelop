/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

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

QxRunner Library
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


namespace QxRunner   // Enables doxygen to create links to documented items
{


/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\mainpage

\image html qxrunner_64x64.png

<!------------------------------------------------------------------->

\section intro Introduction

The QxRunner library helps in developing small C++ GUI applications
where the user can select from a list of items for execution. Each
executed item returns a result which is displayed accordingly. By
subclassing from RunnerModel and RunnerItem it is possible to create
this type of GUI application with little effort.

The library is based on the model/view framework of Qt. It is
recommended to become familiar with this framework before writing
QxRunner applications.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page runner_model_item Runner Model and Runner Item

RunnerModel and RunnerItem are the two classes of most interest for
those wanting to write their own QxRunner based program. By subclassing
these classes and linking with the QxRunner library one has the program
ready.

These sections give some insight into these classes. Please also read
the RunnerModel and RunnerItem description .

<!------------------------------------------------------------------->

\section runner_model The Runner Model

The RunnerModel class implements a tree model with the data contained
in RunnerItem objects. At initialization time the model gets populated
with RunnerItem objects that are linked together in a pointer-based tree
structure or in a less complex non-hierarchical list. Thus RunnerModel
isn't limited to represent tree strucutures only. A simple
non-hierarchical list can be seen as a tree structure with parent items
only that have no child items.

Generally, a RunnerItem has a parent item, and can have a number of
child items. Each RunnerItem object contains information about its place
in the tree structure, it can return its parent item and its row number.
The use of a pointer-based tree structure allows to store the runner
item object pointer in the QModelIndex which refers to the runner item.
Use QModelIndex's \c internalPointer() method to get the pointer.

There must be a root item in the tree structure of the model which is
the top level parent of all runner items. The root item has no parent
item. It is never referenced outside the model but defines the number
of columns and the column headers for the views attached to the model.
Although the root item is automatically assigned a row number of 0,
this information is never used by the model.

The model can run code in the runner items and observe their execution.
Signals are fired to inform the environment about ongoing execution
status. The current model implementation only executes code in items
which have no children, but this could change in the future. Therefore
the code in a runner item should check whether it is a parent or not.
Only selected runner items get executed. Execution is strictly sequential,
i.e. the first runner item found for execution in the tree structure is
run, or if it has children these are run, then the next selected sibling
item is run and so on.

Runner items get executed asynchronously in a separate thread allocated
by RunnerModelThread. Stopping item execution is only possible in the
moment between termination of a runner item and the start of the next one.
The thread isn't stopped the 'hard way' by terminating it which is dangerous
and discouraged in the Qt documentation. Instead a flag is set which
indicates that the thread must stop. This flag is checked regularly in the
method that drives item execution.

Data updates with RunnerModel::setData() are not possible during item
execution. Although the code in runner items can do anything thinkable
it is recommended to keep the code to a minimum and to forego lengthy
operations because a running item can't be interrupted.

Due to the parallelism induced by the thread the item execution must be
synchronized with clients that consume item execution related information.
The GUI for example displays state information about item execution. If
the items are executing faster in the thread than the GUI is able to upate
its widgets in its own thread the information shown in the GUI won't be
reasonable. Therefore the model waits after each executed item until the
signals it emits are completely processed by the signal receivers.

The model can be set to minimal update mode. In this mode only a reduced
amount of item data and execution state information is returned. It's up
to clients to decide when this is useful. QxRunner uses minimal update
mode to speed up item execution because not all item data must be updated
in views for an executing runner item.

Data changes from the outside of the model with RunnerModel::setData()
are allowed for the selected flag in column 0 only. Of course there
occur data changes all the time during item execution but this happens
behind the scenes.

The model name uniquely identifies a model which has its unique structure.
It is used to identify the settings of a program using the model.
Therefore it's not recommended to use a version number or other variable
information as part of the model name. Furthermore it is essential that
the model name can be part of a valid filename, i.e. no special characters
must be used in the name.

\note
 The RunnerModel isn't designed to add additional RunnerItem instances
 to the model after it is constructed and set up.

<!------------------------------------------------------------------->

\subsection results_model The Results Model

The ResultsModel is tightly coupled to the RunnerModel, more
precisely, the RunnerModel class creates the ResultsModel to hold the
results of its runner items and defines the model structure. The
ResultsModel class has mainly been introduced because the RunnerModel
class isn't suitable for displaying chronological data as a simple
non-hierarchical list instead of in a tree structure.

<!------------------------------------------------------------------->

\section runner_item The Runner Item

All runner items have the same number of columns. The first two columns
have a fixed meaning which should not be overriden:

- Column 0 has the name of the item. This column also carries the
  selected flag which indicates whether a runner item is selected for
  execution or not. In views this flag is presented as a checkbox.
- Column 1 is the textual representation of the result after a
  runner item has been executed. In general this result text is set
  by QxRunner but can also be provided otherwise. In the latter case
  the semantical meaning should be that of describing a result type.

The other columns, if any, can be used freely.

\note
 It is important that all RunnerItem objects in RunnerModel
 have the same number of columns otherwise some view manipulations
 could throw assertions as learned by experience. Therefore the
 RunnerItem constructor takes care of the correct number of columns
 for an item.

\sa \ref runner_item_index
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page implementation_ascpects Implementation Aspects

<!------------------------------------------------------------------->

\section settings Settings

Program settings are kept in INI text files. Settings must be managed
for programs using this library and not for the library itself. Therefore
the AppSettings class gives the INI file an unique name based on the
application executable name. Since different executables could use the
same runner model it further makes sense to save settings for a specific
model type rather than for a particular program. It's assumed that users
running different programs with the same model type expect that settings
are applied on a per model type basis.

An application name specific INI file only gets one entry which is the
name of the model used by the program. The model name identifies the INI
file containing the 'real' settings. If no usable model name is available
the settings are kept in the INI file with the executable specific name.

At the moment there's support for only one settings file for a particular
program. This could change in the future if applications get the ability
to support loading different models at run-time.

All INI files created by the AppSettings class are stored in a folder
named \b qxrunner in the user-specific location for INI files on the
target platform.

<!------------------------------------------------------------------->

\section qt_forms Qt Forms

Qt Designer is used for GUI design. Implementation of the forms is done
using the single inheritance approach as described in the Qt documentation.
GUI classes simply call the uic generated base class's constructor and
then set up the form's user interface by calling the Ui object's
setupUi() function.

<!------------------------------------------------------------------->

\section agument_validation Argument Validation

Where applicable the methods of a class perform simple validation of
arguments to prevent from crashes. In case of invalid arguments the
methods either do nothing or return values indicating false or something
similar that fits according to the context.

<!------------------------------------------------------------------->

\section runner_item_index Runner Item Index

When programming with the model/view framework one is confronted with
indexes and items again and again. An index can refer to an item of a
model or a view. To know the meaning of an index depends on the context
where it is used. One cannot deduce from the index variable type to what
kind of item it refers to, since its always a QModelIndex type.

Sometimes it is essential to have a QModelIndex that refers to a
RunnerItem instance. In comments and the documentation the term
<i>runner item index</i> is used to refer to such a QModelIndex.
To make this clear in code the index variable gets named accordingly.
Also methods related to such a QModelIndex have appropriate names, for
example:

\code
QModelIndex runnerItemIndex;
runnerItemIndex = mapToRunnerItemIndex(index(row, 0));
\endcode

The pointer to the RunnerItem object is retrieved with QModelIndex's
\c internalPointer() method:

\code
QModelIndex runnerItemIndex = mapToRunnerItemIndex(index(row, 0));

RunnerItem* item = runnerItemIndex.internalPointer();

if (item->isSelected())
{
 ...
}
else
{
 ...
}
\endcode

An \a item in general is a term used in descriptions or is part of an
abstract concept. The RunnerItem on the other side is a type of this
library. Therefore <i>runner item</i> is used in comments, the
documentation or as part of a variable or method name of this library
when it must be made clear that it has to do with a RunnerItem type.

\note
 A QModelIndex is located in a given row and column in a model.
 If not mentioned otherwise the library code uses the index
 of column 0 for a <i>runner item index</i> returned by a method,
 provided as an argument in a signal or saved for later use.

\sa \ref runner_model_item

<!------------------------------------------------------------------->

\section selected_item Selected Item

A selected item is one that has been chosen from the items presented
in \ref runner_view for execution. But often \a selected means that
something is highlighted in the GUI too. To distinguish between these
two different meanings the term \a highlighted is used in the code to
emphasize when something in the GUI is, well, selected in the meaning
of the latter. For example in method names:

\code
void setHighlightedRow(const QModelIndex& index) const;
\endcode

<!------------------------------------------------------------------->

\section main_window The Main Window

RunnerWindow is the class that implements the main window for a
QxRunner application. It creates the most relevant objects and seams
them together. It's responsible to store the appliciation settings at
program exit and to restore the main window from the settings at next
program start-up.

Users can customize the look of the main window to some extents. On
the other side developers have few possibilites to influence the
initial appearance of the main window. This is on purpose because it's
assumed that it should be the user's choice to define how the window
appears.

The main window adapts to its model by providing just those interface
elements that make sense. If a model, for example, has no runner items
that return a QxRunner::RunInfo result the button for filtering info
results isn't visible. If there is no valid model then nearly all user
interaction elements are disabled.

The main window starts item execution in the RunnerModel and prevents
user interactions that could distract the ongoing item execution.

\note
 The statusbar shows counters for the number of different result
 types returned from runner items. Even when a particular result
 type isn't expected it will be shown in the statusbar. This can
 give a hint to the developer of a QxRunner application that the
 runner item or the model code must be corrected.

\sa \ref result_types

<!------------------------------------------------------------------->

\section views Views, View Controllers and Proxy Models

There are 2 views in the main window which represent data from the
models. The views always have as many columns as defined in the model
they are representing. There are only horizontal headers for the views.

The views represent the data of the model they are attached to either
as a simple non-hierarchical list of items without children or as
a hierarchical tree structure where items that have children can be
expanded (children are visible) or collapsed (children are hidden).

Users can define the column visibilities in the ColumnsDialog. Column 0
is the most important one and cannot be hidden. Hiding or showing of a
column is done by disabling or enabling it in the proxy model as described
under \ref proxy_models.

\subsection runner_view The Runner View

The runner view is attached to a RunnerProxyModel which in turn has
a RunnerModel as its source model. The view represents the runner
items according to the tree structure defined in the model. The items
to be executed are selected in the runner view. The selected flag
is represented with a checkbox. Parent item checkboxes are tri-state
to show whether all, none or some of its children are selected. Fine
grained control over the view is implemented in RunnerViewController.

Sorting of columns isn't supported in the runner view.

\subsection results_view The Results View

The results view is attached to a ResultsProxyModel which in turn has
a ResultsModel as its source model. The view represents the runner item
results as a simple non-hierarchical list of items without children.
Results of certain types can be filtered out in the results view. Fine
grained control over the view is implemented in ResultsViewController.

The results view allows sorting of columns  when there is data in the
results model. If sorting is enabled the sort indicator in the header of
the active column is shown and the items are sorted in the corresponding
sort order. When the results get removed from the results model sorting
also gets disabled.

\subsection view_controllers View Controllers

Due to the chosen approach for GUI design (see \ref qt_forms) it was
not appropriate to subclass from QTreeView to implement view specific
behaviour. Instead composition was chosen by introducing view controller
classes which acquire a reference to the view object at run-time.

Controller classes expect that the controlled view is attached to a model.
Specific view controllers subclass ViewControllerCommon.

\subsection proxy_models Proxy Models

Views in QxRunner access data through a sorting filter proxy model
which refers to the 'real' data in a source model.

No column of a model ever gets filtered out because this would change
the structure of the model for a view. Therefore a view always has
as many columns as defined in the source model of a proxy model which
greatly simplifies view handling. But columns can be 'disabled' which
is done by maintaing a flag for each column. Disabled columns are not
visible in a view (which must be guaranteed by the GUI classes that
control a view). Therefore no data should be returned for disabled
columns to improve performance. This must be guaranteed by the sorting
filter model classes which derive from ProxyModelCommon.

A proxy model can be set to active or inactive. How this is used depends
on the classes that subclass ProxyModelCommon which merely maintains the
state of the active flag.

<!------------------------------------------------------------------->

\section symbols Exported Symbols

This applies for the Win32 world only. By default symbols of the library
are 'hidden' when created as a DLL. Therefore symbols, in particular
classes, that must be accessible from the outside of the library are
exported with the QXRUNNER_EXPORT macro:

\code
class QXRUNNER_EXPORT RunnerItem
{
public: // Operations
 ...
\endcode

Only a few classes are exported right now, the other classes are for
internal use only. This might be reconsidered in the future and more
classes could become exported to be used somewhere outside the library.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page result_types Result Types

At the end of its execution a runner item returns a result code to report
the execution status. The result code is of type QxRunner::RunnerResult.
This set of predefined result types is inspired by good old
<a target="_blank" href="http://h71000.www7.hp.com/DOC/82final/5973/5973pro_022.html">
VMS condition handling</a>.

All result types except QxRunner::RunSuccess and QxRunner::RunException
should have an accompanying result message which describes the reason for
the result. A RunnerModel knows which result types to expect from its runner
items. The GUI can retrieve the expected result set from the model and adjust
its appearance accordingly. Developers of a new model may limit the expected
results to a subset of QxRunner::RunnerResult.

\note
 Runner items shouldn't return the QxRunner::RunException code. It
 is used by QxRunner to indicate failed execution of a runner item.
 Exceptions aren't filtered out by ResultsProxyModel since they
 mostly turn out to be a programming error in RunnerItem::run().
 Developers should patch that code by implementing a reasonable
 error handling.
 <br>
 <br>
 A further developed version of the library could also provide a
 result message reporting the error reason for an exception.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\namespace QxRunner

\brief Namespace for the QxRunner library entities.

\namespace Ui

\brief Generated GUI components expose their widgets and layouts in the
       Ui namespace.

The Ui namespace is defined by the uic generated files for the forms
created with Qt Designer. It contains all the widgets and layouts used
in the forms which expose their elements via the Ui namespace to other
objects in the project.
*/

/*!<!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-->*/

/*!
\page examples Examples

The demo program shipped with the QxRunner library can be used as the starting
point for developping a QxRunner based program. It subclasses RunnerModel and
RunnerItem and uses a Runner instance in the main program to launch the
GUI. The model constructs a tree structure of runner items which randomly
generate a result of type QxRunner::RunnerResult. An item also reports the
location of where in the code the result was created and supplies a related
result message.

\sa
For a deeper insight into the details of the model/view framework and how to
create a basic model to use with Qt's standard view classes see the
<i>Simple Tree Model Example</i> provided with the Qt installation. And, yes,
much of this example documentation uses slightly modified text from that Qt
example.

\section demo_model The Demo Model

\subsection demo_model_header Demo Model Header File

The constructor takes an argument containing the data that the model will
share with views. The \c name() method returns the model name used by QxRunner
for display in the about box and as the identifier for the settings file. The
model's internal data structure is populated with \c DemoItem instances by the
\c setupModelData() helper method. The model isn't designed to add data
to the model after it is constructed and set up.

\includelineno demomodel.h

\subsection demo_model_impl Demo Model Implementation File

The constructor creates the root item for the model. This item only contains
horizontal header data. It is also used to reference the internal data structure
that contains the model data, and it is used to represent an imaginary parent
of top-level items in the model. The model's internal data structure is
populated with items by the \c setupModelData() function. The destructor does
nothing, it relies on the parent RunnerModel destructor who deletes the root
item and all of its descendants when the model is destroyed.

The \c setupModelData() method sets up the initial data in the model. It
iterates over a list of 'keywords' which define the position of an item in the
tree strucuture and the related item name. Of course this is a very simple model
and is only intended for demonstrational purposes.

The keyword meanings are:

- \b L0 defines a top level item.
- \b L1 defines an item which is a child of a L0 item and itself has child
        items, thus starts a new branch.
- \b L2 defines an item which is a child of a L1 item and itself has child
        items, thus starts a new branch.
- \b L3 defines an item which is a child of a L2 item and itself has child
        items, thus starts a new branch.
- \b CH defines a child item which has no child items of itws own (a leaf). It
        is appended as a child to the last parent item encountered in the list.

\includelineno demomodel.cpp

\section demo_item The Demo Item

\subsection demo_item_header Demo Item Header File

The constructor is used to record the item's parent and the data associated with
each column. The abstract \c run() method from the parent RunnerItem class must be
reimplemented and contains the custom code that gets executed when the items are
run by QxRunner.

\includelineno demoitem.h

\subsection demo_item_impl Demo Item Implementation File

The constructor simply calls the parent RunnerItem constructor. The destructor does
nothing, it relies on the parent RunnerItem destructor who ensures that all child
items get deleted.

The \c run() method first checks whether it is a parent item or not (see
\ref runner_model). Only leafs have custom logic for execution. If it is a child
the method randomly creates a result type and fills related data into the item columns.
A special case is the <i>unhandled exception</i> which gets caught by the model. The
commented out code at the beginning was used during development to slow down item
execution.

\includelineno demoitem.cpp

\section demo_main The Demo Main Program

The main program is straightforward:

\includelineno main.cpp
*/

} // namespace
