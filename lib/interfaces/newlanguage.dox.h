/** \file newlanguage.dox.h
  * \brief How to add support for a language in KDevelop
  */

/** \page howToAddNewLanguage How to add support for a language in KDevelop

Before beginning:  To add support for a language in KDevelop, you need
to be more than passingly familiar with C++, language parsing,
the language you are adding, and KDE/QT.  This is not something that
an beginner developer should try to complete.  However, you *can*
get minimal language support without too much trouble.  That said, lets
move on...

List of things to have "complete" support of a given language in KDevelop:

  - Implement interface KDevlLanguageSupport
  - Language parser for class browser
  - UI subclassing (if the language has Qt bindings)
  - Problem reporter (parses source on the fly and reports syntax errors)
  - Syntax highlighter (add to QEditor if not available elsewhere)
  - Project manager (make/ant/etc)
  - Source file templates
  - Application wizard
  - Source code abbreviations  (ife expands to an if else statement, etc)
  - Source formatter (prettyprint functionality)
  - Debugger (gdb/jdb/??? integration)
  .

Explanations, hints, tips, and all around Good Things To Know, regarding
that list:

Any language support should be written as a kdevelop part and implement
KDevLanguageSupport interface (lib/interfaces/kdevlanguagesupport.h).
Implementing methods:
    virtual Features features();
    virtual KMimeType::List mimeTypes();
should be enough for a language support to start working.  A KDevelop
part is available as a New Project type: C++->KDevelop->KDevelop Part.
FIXME: CAN YOU USE EITHER STANDALONE OR SOURCE TREE BUILD, OR DO YOU
NEED ONE IN PARTICULAR?

You should look at parts/rubysupport for a simple language support
implementation. For a compilable language support, consult parts/adasupport
or parts/pascalsupport (they are not so complex as cppsupport).  Pascal
would be a good starting place, as it is the smaller of the two by far.

Language support can offer additional features:
new class wizard: (See cppsupport)
    virtual void addClass();
add method dialog: (See adasupport or cppsupport)
    virtual void addMethod(const QString &className);
add attribute dialog: (See cppsupport)
    virtual void addAttribute(const QString &className);

If there is a Qt bindings for your language and there is a possibility
to use QtDesigner ui files, you could implement ui subclassing feature:
    virtual QStringList subclassWidget(const QString& formName);
    virtual QStringList updateWidget(const QString& formName, const
QString& fileName);
See cppsupport and javasupport for examples.

If you write (or have) a language parser, your language support can have
"class store" (a database containing the information about scopes, classes
and methods - their names, names of source files, location in source files,
etc.). Class store libraries can be found at lib/catalog and
lib/sourceinfo. KDevelop provides class browsers that extract information
from a class store and display it in a tree view and toolbar selectors of
scopes, classes and methods.

Catalog is the new persistant class store for KDevelop written by Roberto
Raggi and everybody is highly encouraged to use it. Take a look at
parts/cppsupport for an example of using catalog. parts/pascalsupport will
be based on a catalog (check it out within a two weeks (06/26/03). Catalog
is stored on disk in the database file (Berkeley db) If you use catalog,
your class browser will be parts/classbrowser.

Sourceinfo is the deprecated class store. Look at parts/adasupport
(especially interesting is ada.store.g grammar file) to find out how it is
filled by a parser with information. This class store can't be saved onto a
disk. The class browser for a sourceinfo based stores is parts/classview.

Class store enables you to write a code completion for the language. At the
moment (06/25/03), code completion is available only to cppsupport so take
a look at it for an example.

In general, class stores can be filled with information without specialized
and complex language parsers (take a look at parts/pythonsupport that have
a very simple python parser) but your language support will surely benefit
from having such. There is a hand-written c/c++ parser (lib/cppparser) in
KDevelop that might be used for ObjC or related C-based languages. Other
(not so complex as c++) languages can be parsed by ANTLR based parsers
(library is in lib/antlr). Consult www.antlr.org for a ANTLR documentation
and look at parts/javasupport, parts/adasupport and parts/pascalsupport for
an example of using such parsers. The latest version of ANTLR (2.7.2)
has support for Java, C, Pascal, Ada, C++, CIM, HTML, IDL, Verilog,
VRML, OCL, ASN.1, and SQL.  You can write an ANTLR parser for your own
language, of course.

If you have a language parser, you can implement "problem reporter"
functionality for your language. The problem reporter catches errors
reported by a parser and displays it in a problem reporter view.
parts/javasupport, parts/adasupport, parts/pascalsupport and
parts/cppsupport have problem reporters.

The language support is important, but it is unusable without a project
manager that can manage projects written on this language. KDevelop
currently provides several project managers. They are:
Automake manager        parts/autoproject
QMake manager       parts/trollproject
Custom project manager  parts/customproject
    (works with custom makefiles, also has ant support)
Script project manager  parts/scriptproject
    (the generic project manager for all scripting languages).
Also available parts/pascalproject and parts/adaproject (they are the
project managers for a compiled language with main source file concept).
Choose your project manager and if the existing project managers don't fit
in, you can modify parts/customproject to use a specific build tool or help
us to develop "generic project manager" suitable for every language and
build tool (there are some ideas which haven't been implemented yet).

Application wizard templates should be also written. Appwizard templates
are simple to create - consult parts/appwizard/README and look at
parts/appwizard/rubyhello, parts/appwizard/pascalhello or
parts/appwizard/adahello.

Another thing to do is to create filecreate templates. They are prototypes
for a source files of your language. These prototypes are placed in
parts/filecreate/file-templates dir and have names equal to the extensions
of language source files. The description of the prototypes is placed
in parts/filecreate/template-info.xml. Consult parts/filecreate/README
for further information.

KDevelop has a support for code abbrevations so you can add some predefined
abbrevations to your language support. Take parts/cppsupport/cpptemplates
as an example.

To edit source files KDevelop uses any editor that supports KTextEditor
interface. In case none from the editors support advanced editing of
sources written on your language (like code folding, syntax highlighting,
line indentation) you can improve QEditor included in KDevelop
(parts/qeditor). By creating QEditorIndenter and QSourceColorizer
descendants you can provide the support for an automatic indentation and
syntax highlighting that will be available for sure in KDevelop.

To obtain source formatter functionality (that is already available to
c-based languages) you can improve astyle library (lib/astyle) that is used
by KDevelop to format sources.

The last thing to have a complete language support in KDevelop is to
write a debugger support. KDevelop already provides GDB support
(parts/debugger) and JDB (java debugger) support (parts/javadebugger).
Take a look at them to get inspiration.

See also HACKING file for an information on how to create a generic
KDevelop plugin and how to manage project and global configuration
information.

*/
