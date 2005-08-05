#include <qmap.h>


#include <kdebug.h>
#include <klocale.h>


static QMap<QCString,QString> messages;


// Note: this function seems to be completely unnecessary, but messages.insert
// is a template function, so doing the template expansion just once reduced
// the compile time of this trivial file from >5m to <30s on my machine. mhk.
static void addMessage(const QCString key, const QString &message)
{
  messages.insert(key, message);
}


QString message(const QCString &key)
{
  static bool initialized = false;

  if (!initialized)
  {
    //---------------------------------------------------------------------------
    // Project related configuration options
    //---------------------------------------------------------------------------
    addMessage("Project", i18n("Project"));
    addMessage("PROJECT_NAME", i18n("Project name"));
    addMessage("PROJECT_NUMBER", i18n("Project version"));
    addMessage("OUTPUT_DIRECTORY", i18n("Output path"));
    addMessage("CREATE_SUBDIRS", i18n("Distribute the generated docs in 10 subdirectories"));
    addMessage("OUTPUT_LANGUAGE", i18n("Output language"));
    addMessage("USE_WINDOWS_ENCODING", i18n("Use MS Windows(tm) font encoding"));
    addMessage("BRIEF_MEMBER_DESC", i18n("Include brief member descriptions"));
    addMessage("REPEAT_BRIEF", i18n("Repeat brief member descriptions"));
    addMessage("ABBREVIATE_BRIEF", i18n("Strip words in brief description"));
    addMessage("ALWAYS_DETAILED_SEC", i18n("Always include detailed section"));
    addMessage("INLINE_INHERITED_MEMB", i18n("Inline inherited members"));
    addMessage("FULL_PATH_NAMES", i18n("Always use full path names"));
    addMessage("STRIP_FROM_PATH", i18n("Prefix to strip from path names"));
    addMessage("STRIP_FROM_INC_PATH", i18n("Strip from include path"));
    addMessage("SHORT_NAMES", i18n("Generate short file names"));
    addMessage("JAVADOC_AUTOBRIEF", i18n("Use JavaDoc-style brief descriptions"));
    addMessage("MULTILINE_CPP_IS_BRIEF", i18n("Multiline cpp is brief"));
    addMessage("DETAILS_AT_TOP", i18n("Details at top"));
    addMessage("INHERIT_DOCS", i18n("Inherit documentation"));
    addMessage("DISTRIBUTE_GROUP_DOC", i18n("Use group documentation on undocumented members"));
    addMessage("TAB_SIZE", i18n("Tab size"));
    addMessage("ALIASES", i18n("Aliases"));
    addMessage("OPTIMIZE_OUTPUT_FOR_C", i18n("Optimize output for C"));
    addMessage("OPTIMIZE_OUTPUT_JAVA", i18n("Optimize output for Java"));
    addMessage("SUBGROUPING", i18n("Class members type subgrouping"));

    //---------------------------------------------------------------------------
    // Build related configuration options
    //---------------------------------------------------------------------------
    addMessage("Build", i18n("Build"));
    addMessage("EXTRACT_ALL", i18n("Extract undocumented entities"));
    addMessage("EXTRACT_PRIVATE", i18n("Extract private entities"));
    addMessage("EXTRACT_STATIC", i18n("Extract static entities"));
    addMessage("EXTRACT_LOCAL_CLASSES", i18n("Extract local classes"));
    addMessage("EXTRACT_LOCAL_METHODS", i18n("Extract local methods"));
    addMessage("HIDE_UNDOC_MEMBERS", i18n("Hide undocumented members"));
    addMessage("HIDE_UNDOC_CLASSES", i18n("Hide undocumented classes"));
    addMessage("HIDE_FRIEND_COMPOUNDS", i18n("Hide friend compounds"));
    addMessage("HIDE_IN_BODY_DOCS", i18n("Hide in body docs"));
    addMessage("INTERNAL_DOCS", i18n("Document internal entities"));
    addMessage("CASE_SENSE_NAMES", i18n("Use case-sensitive file names"));
    addMessage("HIDE_SCOPE_NAMES", i18n("Hide name scopes"));
    addMessage("SHOW_INCLUDE_FILES", i18n("Show included files"));
    addMessage("INLINE_INFO", i18n("Make inline functions"));
    addMessage("SORT_MEMBER_DOCS", i18n("Sort member documentation alphabetically"));
    addMessage("SORT_BY_SCOPE_NAME", i18n("Sort the class list by fully-qualified names"));
    addMessage("SORT_BRIEF_DOCS", i18n("Sort brief documentation alphabetically"));
    addMessage("ENABLED_SECTIONS", i18n("Enable conditional sections"));
    addMessage("GENERATE_TODOLIST", i18n("Generate TODO-list"));
    addMessage("GENERATE_TESTLIST", i18n("Generate Test-list"));
    addMessage("GENERATE_BUGLIST", i18n("Generate Bug-list"));
    addMessage("GENERATE_DEPRECATEDLIST", i18n("Generate Deprecated-list"));
    addMessage("MAX_INITIALIZER_LINES", i18n("Maximum lines shown for initializers"));
    addMessage("SHOW_USED_FILES", i18n("Show used files"));

    //---------------------------------------------------------------------------
    // configuration options related to warning and progress messages
    //---------------------------------------------------------------------------
    addMessage("Messages", i18n("Messages"));
    addMessage("QUIET", i18n("Suppress output"));
    addMessage("WARNINGS", i18n("Show warnings"));
    addMessage("WARN_IF_UNDOCUMENTED", i18n("Warn about undocumented entities"));
    addMessage("WARN_IF_DOC_ERROR", i18n("Warn if error in documents"));
    addMessage("WARN_FORMAT", i18n("Warnings format"));
    addMessage("WARN_LOGFILE", i18n("Write warnings to"));

    //---------------------------------------------------------------------------
    // configuration options related to the input files
    //---------------------------------------------------------------------------
    addMessage("Input", i18n("Input"));
    addMessage("INPUT", i18n("Input files and directories"));
    addMessage("FILE_PATTERNS", i18n("Input patterns"));
    addMessage("RECURSIVE", i18n("Recurse into subdirectories"));
    addMessage("EXCLUDE", i18n("Exclude from input"));
    addMessage("EXCLUDE_SYMLINKS", i18n("Exclude symlinks"));
    addMessage("EXCLUDE_PATTERNS", i18n("Exclude patterns"));
    addMessage("EXAMPLE_PATH", i18n("Path to examples"));
    addMessage("EXAMPLE_PATTERNS", i18n("Example patterns"));
    addMessage("EXAMPLE_RECURSIVE", i18n("Example recursive"));
    addMessage("IMAGE_PATH", i18n("Path to images"));
    addMessage("INPUT_FILTER", i18n("Input filter"));
    addMessage("FILTER_SOURCE_FILES", i18n("Filter input files"));
    addMessage("FILTER_PATTERNS", i18n("Apply filters on file patterns"));

    //---------------------------------------------------------------------------
    // configuration options related to source browsing
    //---------------------------------------------------------------------------
    addMessage("Source Browser", i18n("Source Browser"));
    addMessage("SOURCE_BROWSER", i18n("Cross-reference with source files"));
    addMessage("INLINE_SOURCES", i18n("Inline sources"));
    addMessage("STRIP_CODE_COMMENTS", i18n("Hide special comment blocks"));
    addMessage("REFERENCED_BY_RELATION", i18n("Referenced by relation"));
    addMessage("REFERENCES_RELATION", i18n("References relation"));
    addMessage("VERBATIM_HEADERS", i18n("Include headers verbatim"));

    //---------------------------------------------------------------------------
    // configuration options related to the alphabetical class index
    //---------------------------------------------------------------------------
    addMessage("Index", i18n("Index"));
    addMessage("ALPHABETICAL_INDEX", i18n("Generate alphabetical index"));
    addMessage("COLS_IN_ALPHA_INDEX", i18n("Columns in index"));
    addMessage("IGNORE_PREFIX", i18n("Prefix to ignore"));

    //---------------------------------------------------------------------------
    // configuration options related to the HTML output
    //---------------------------------------------------------------------------
    addMessage("HTML", i18n("HTML"));
    addMessage("GENERATE_HTML", i18n("Generate HTML"));
    addMessage("HTML_OUTPUT", i18n("HTML output directory"));
    addMessage("HTML_FILE_EXTENSION", i18n("HTML file extension"));
    addMessage("HTML_HEADER", i18n("Header file"));
    addMessage("HTML_FOOTER", i18n("Footer file"));
    addMessage("HTML_STYLESHEET", i18n("Stylesheet"));
    addMessage("HTML_ALIGN_MEMBERS", i18n("Align members"));
    addMessage("GENERATE_HTMLHELP", i18n("Generate HTML-help"));
    addMessage("CHM_FILE", i18n("CHM file"));
    addMessage("HHC_LOCATION", i18n("HHC location"));
    addMessage("GENERATE_CHI", i18n("Generate .chi file"));
    addMessage("BINARY_TOC", i18n("Generate binary TOC"));
    addMessage("TOC_EXPAND", i18n("Expand TOC"));
    addMessage("DISABLE_INDEX", i18n("Disable index"));
    addMessage("ENUM_VALUES_PER_LINE", i18n("Number of enum values per line"));
    addMessage("GENERATE_TREEVIEW", i18n("Generate treeview"));
    addMessage("TREEVIEW_WIDTH", i18n("Treeview width"));

    //---------------------------------------------------------------------------
    // configuration options related to the LaTeX output
    //---------------------------------------------------------------------------
    addMessage("LaTeX", i18n("LaTeX"));
    addMessage("GENERATE_LATEX", i18n("Generate LaTeX"));
    addMessage("LATEX_OUTPUT", i18n("LaTeX output directory"));
    addMessage("LATEX_CMD_NAME", i18n("LaTeX command name"));
    addMessage("MAKEINDEX_CMD_NAME", i18n("MakeIndex command name"));
    addMessage("COMPACT_LATEX", i18n("Generate compact output"));
    addMessage("PAPER_TYPE", i18n("Paper type"));
    addMessage("EXTRA_PACKAGES", i18n("Include extra packages"));
    addMessage("LATEX_HEADER", i18n("LaTeX Header file"));
    addMessage("PDF_HYPERLINKS", i18n("Generate PDF hyperlinks"));
    addMessage("USE_PDFLATEX", i18n("Use pdflatex"));
    addMessage("LATEX_BATCHMODE", i18n("Use batch mode"));
    addMessage("LATEX_HIDE_INDICES", i18n("Do not include index chapters"));

    //---------------------------------------------------------------------------
    // configuration options related to the RTF output
    //---------------------------------------------------------------------------
    addMessage("RTF", i18n("RTF"));
    addMessage("GENERATE_RTF", i18n("Generate RTF"));
    addMessage("RTF_OUTPUT", i18n("RTF output directory"));
    addMessage("COMPACT_RTF", i18n("Generate compact output"));
    addMessage("RTF_HYPERLINKS", i18n("Generate hyperlinks"));
    addMessage("RTF_STYLESHEET_FILE", i18n("Stylesheet"));
    addMessage("RTF_EXTENSIONS_FILE", i18n("Extensions file"));

    //---------------------------------------------------------------------------
    // configuration options related to the man page output
    //---------------------------------------------------------------------------
    addMessage("Man", i18n("Man"));
    addMessage("GENERATE_MAN", i18n("Generate man pages"));
    addMessage("MAN_OUTPUT", i18n("Man output directory"));
    addMessage("MAN_EXTENSION", i18n("Man extension"));
    addMessage("MAN_LINKS", i18n("Generate links"));

    //---------------------------------------------------------------------------
    // configuration options related to the XML output
    //---------------------------------------------------------------------------
    addMessage("XML", i18n("XML"));
    addMessage("GENERATE_XML", i18n("Generate XML"));
    addMessage("XML_OUTPUT", i18n("XML output directory"));
    addMessage("XML_SCHEMA", i18n("XML schema"));
    addMessage("XML_DTD", i18n("XML DTD file"));
    addMessage("XML_PROGRAMLISTING", i18n("Dump program listings to the XML output"));

    //---------------------------------------------------------------------------
    // configuration options for the AutoGen Definitions output
    //---------------------------------------------------------------------------
    addMessage("DEF", i18n("DEF"));
    addMessage("GENERATE_AUTOGEN_DEF", i18n("Generate Autogen DEF"));

    //---------------------------------------------------------------------------
    // configuration options related to the Perl module output
    //---------------------------------------------------------------------------
    addMessage("PerlMod", i18n("Perl"));
    addMessage("GENERATE_PERLMOD", i18n("Generate Perl module"));
    addMessage("PERLMOD_LATEX", i18n("Generate PDF and DVI using LaTeX"));
    addMessage("PERLMOD_PRETTY", i18n("Make the Perl module output human readable"));
    addMessage("PERLMOD_MAKEVAR_PREFIX", i18n("Perl module variable prefix in the makefiles"));

    //---------------------------------------------------------------------------
    // Configuration options related to the preprocessor
    //---------------------------------------------------------------------------
    addMessage("Preprocessor", i18n("Preprocessor"));
    addMessage("ENABLE_PREPROCESSING", i18n("Enable preprocessing"));
    addMessage("MACRO_EXPANSION", i18n("Expand macros"));
    addMessage("EXPAND_ONLY_PREDEF", i18n("Expand only predefined macros"));
    addMessage("SEARCH_INCLUDES", i18n("Search for includes"));
    addMessage("INCLUDE_PATH", i18n("Include path"));
    addMessage("INCLUDE_FILE_PATTERNS", i18n("Include file patterns"));
    addMessage("PREDEFINED", i18n("Predefined macros"));
    addMessage("EXPAND_AS_DEFINED", i18n("Expand as defined"));
    addMessage("SKIP_FUNCTION_MACROS", i18n("Skip function macros"));

    //---------------------------------------------------------------------------
    // Configuration::addtions related to external references
    //---------------------------------------------------------------------------
    addMessage("External", i18n("External"));
    addMessage("TAGFILES", i18n("Tagfiles"));
    addMessage("GENERATE_TAGFILE", i18n("Generate tagfile"));
    addMessage("ALLEXTERNALS", i18n("List all externals"));
    addMessage("EXTERNAL_GROUPS", i18n("External groups"));
    addMessage("PERL_PATH", i18n("Path to Perl"));

    //---------------------------------------------------------------------------
    // Configuration options related to the dot tool
    //---------------------------------------------------------------------------
    addMessage("Dot", i18n("Dot"));
    addMessage("CLASS_DIAGRAMS", i18n("Generate class diagrams"));
    addMessage("HIDE_UNDOC_RELATIONS", i18n("Hide undocumented relations"));
    addMessage("HAVE_DOT", i18n("Use dot"));
    addMessage("CLASS_GRAPH", i18n("Generate class graphs"));
    addMessage("COLLABORATION_GRAPH", i18n("Generate collaboration graphs"));
    addMessage("UML_LOOK", i18n("Generate UML looking inheritance and colloboration diagrams"));
    addMessage("TEMPLATE_RELATIONS", i18n("Show relations between templates and their instances"));
    addMessage("INCLUDE_GRAPH", i18n("Generate include graphs"));
    addMessage("INCLUDED_BY_GRAPH", i18n("Generate included-by graphs"));
    addMessage("CALL_GRAPH", i18n("Generate a call dependency graph for every global function or class method"));
    addMessage("GRAPHICAL_HIERARCHY", i18n("Generate graphical hierarchy"));
    addMessage("DOT_IMAGE_FORMAT", i18n("Dot image format"));
    addMessage("DOT_PATH", i18n("Path to dot"));
    addMessage("DOTFILE_DIRS", i18n("Directories with extra dot files"));
    addMessage("MAX_DOT_GRAPH_WIDTH", i18n("Maximum graph width"));
    addMessage("MAX_DOT_GRAPH_HEIGHT", i18n("Maximum graph height"));
    addMessage("MAX_DOT_GRAPH_DEPTH", i18n("Maximum graph depth"));
    addMessage("GENERATE_LEGEND", i18n("Generate legend"));
    addMessage("DOT_CLEANUP", i18n("Remove intermediate files"));
    addMessage("Search", i18n("Search"));

    //---------------------------------------------------------------------------
    // Configuration::addtions related to the search engine
    //---------------------------------------------------------------------------
    addMessage("SEARCHENGINE", i18n("Search engine"));
    addMessage("CGI_NAME", i18n("CGI name"));
    addMessage("CGI_URL", i18n("CGI URL"));
    addMessage("DOC_URL", i18n("Documentation URL"));
    addMessage("DOC_ABSPATH", i18n("Absolute path to documentation"));
    addMessage("BIN_ABSPATH", i18n("Absolute path to doxysearch binary"));
    addMessage("EXT_DOC_PATHS", i18n("Paths to external documentation"));

    initialized = true;
  }

  if (!messages.contains(key))
  {
    kdDebug() << "Unhandled message: " << key << endl;

    return key;
  }

  return messages[key];
}

