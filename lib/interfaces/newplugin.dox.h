/** \file newplugin.dox.h
  * \brief How to extend KDevelop via plugins
  */

/** \page howToExtend How to extend KDevelop via plugins

\section createDesktop Step 1: Make your plugin loadable

For a plugin <code>foo</code>, create a file <code>foo.desktop</code> which contains KDevelop/Part in its list of ServiceTypes.

  - See <code>parts/doctreeview/kdevdoctreeview.desktop</code> for an example.
  .

If you install this file into <code>\$(kde_servicesdir)</code>, your plugin will automatically be loaded.

\subsection changeLoading How to change the default loading

You can change the default loading by changing some settings in your <code>foo.desktop</code> file:

  - Set <code>X-KDevelop-Scope=</code> to <code>Global</code> or
    <code>Project</code>
    - <b>Note:</b> This property is <i>not</i> optional
	.
  - You can add a list of programming languages which are supported by your
    plugin
	- If your plugin works with all languages leave the
	  <code>X-KDevelop-ProgrammingLanguages=</code> field empty
	  <i>(optional)</i>
	.
  - You can add a list of keywords.
    - The plugin will only be loaded if all keywords match with the
	<code>Keywords=</code> field in the projectfile <i>(optional)</i>.
	.
  .

An example from the Java Debugger Plugin:

<code><pre>
    #######################
    X-KDevelop-Scope=Project
    X-KDevelop-ProgrammingLanguages=Java
    Keywords=
    ##########################
</pre></code>
    

\section createFactory Step 2: Make the plugin accessible by the factory

Create a factory class <code>FooFactory</code> which inherits
<code>KDevFactory</code>. Put a section

<code><pre>
    extern "C" {
        void *init_libfoo()
        {
            return new FooFactory;
        }
    }
</pre></code>

into the source file, so that the factory can be accessed by KDE's library
loader. Keep in mind that the name of the method <code>init_libfoo()</code> is
required for a library with the name </code>libfoo.so<code>.

This may be simplified by the use of the
<code>K_EXPORT_COMPONENT_FACTORY</code> macro which is defined in
<code>klibloader.h</code>:

<code>
K_EXPORT_COMPONENT_FACTORY( libfoo, FooFactory );
</code>

  - <i>Note:</i> Your factory must reimplement the
    <code>createPartObject()</code> method of <code>KDevFactory</code> and
	produce the part there.
  .

See <code>parts/doctreeview/doctreeviewfactory.cpp</code> for an example.


\section implementPart Step 3: Implement your part.

Your part must be derived from <code>KDevPlugin</code>.

  - KDevPlugin takes two arguments:
    - 1) A <i>parent</i> argument. This also comes from
	  <code>createPartObject()</code>.
	- 2) A <i>name</i>, which in turn is given to the <code>QObject</code>
	  constructor.
	.
  .

\subsection accessIDE How to access other IDE components

A part can access other components of the IDE via some accessors
of <code>KDevPlugin</code>:

  - The <i>application core</i> via <code>core()</code>,
  - the <i>project management</i> via <code>project()</code>,
  - the <i>programming language specific stuff</i> via
    <code>languageSupport()</code>,
  - the <i>make frontend</i> via <code>makeFrontend()</code>,
  - the part which displays <i>appication output</i> via
    <code>appFrontend()</code>,
    and finally
  - the <i>symbol database</i> via <code>classStore()</code>.
  .

In order to see what these components provide, see <code>lib/interfaces/kdev*.h</code>.

\subsection userPrefs How to store user preferences

Parts can also store user preferences on a per-project basis. To this
end, they can access a <code>QDomDocument</code> representing the project file
(which is stored as xml) via <code>document()</code>.

For your convenience, you don't have to use the quite complex DOM API. Strings
can very easily be read from and written to this document using the
<code>DomUtil</code> class. Here, entries are identified by a 'path' in the
document. You can think of the DOM document as representing a file system
rooted in the <code>dom</code> document node.

For example, the <code>autoproject</code> part uses the statement

<code><pre>
    QString cflags = DomUtil::readEntry( *part->document(),
                                         "/kdevautoproject/cflags" );
</pre></code>

to read the <code>CFLAGS</code> variable set by the user, and uses the statement similar to

<code><pre>
    DomUtil::writeEntry( *part->document(),
                         "kdevautoproject/cflags",
                         "--no-exceptions" );
</pre></code>

to write it back.

  - <i>Note:</i> In order to avoid conflicts between different plugins, you
    should use your part name as top-level 'directory' in the configuration
	tree.
  .

*/

