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
    addMessage("General", i18n("General"));
    addMessage("PROJECT_NAME", i18n("Project name"));
    addMessage("PROJECT_NUMBER", i18n("Project version"));
    addMessage("OUTPUT_DIRECTORY", i18n("Output path"));
    addMessage("OUTPUT_LANGUAGE", i18n("Output language"));
    addMessage("EXTRACT_ALL", i18n("Extract undocumented entities"));
    addMessage("EXTRACT_PRIVATE", i18n("Extract private entities"));
    addMessage("EXTRACT_STATIC", i18n("Extract static entitites"));
    addMessage("HIDE_UNDOC_MEMBERS", i18n("Hide undocumented members"));
    addMessage("HIDE_UNDOC_CLASSES", i18n("Hide undocumented classes"));
    addMessage("BRIEF_MEMBER_DESC", i18n("Include brief member descriptions"));
    addMessage("REPEAT_BRIEF", i18n("Repeat brief member descriptions"));
    addMessage("ALWAYS_DETAILED_SEC", i18n("Always include detailed section"));
    addMessage("FULL_PATH_NAMES", i18n("Always use full path names"));
    addMessage("STRIP_FROM_PATH", i18n("Prefix to strip from path names"));
    addMessage("INTERNAL_DOCS", i18n("Document internal entities"));
    addMessage("CLASS_DIAGRAMS", i18n("Generate class diagrams"));
    addMessage("SOURCE_BROWSER", i18n("Cross-reference with source files"));
    addMessage("INLINE_SOURCES", i18n("Inline sources"));
    addMessage("STRIP_CODE_COMMENTS", i18n("Hide special comment blocks"));
    addMessage("CASE_SENSE_NAMES", i18n("Use case-sensitive file names"));
    addMessage("SHORT_NAMES", i18n("Generate short file names"));
    addMessage("HIDE_SCOPE_NAMES", i18n("Hide name scopes"));
    addMessage("VERBATIM_HEADERS", i18n("Include headers verbatim"));
    addMessage("SHOW_INCLUDE_FILES", i18n("Show included files"));
    addMessage("JAVADOC_AUTOBRIEF", i18n("Use JavaDoc-style brief descriptions"));
    addMessage("INHERIT_DOCS", i18n("Inherit documentation"));
    addMessage("INLINE_INFO", i18n("Make inline functions"));
    addMessage("SORT_MEMBER_DOCS", i18n("Sort member documentation alphabetically"));
    addMessage("DISTRIBUTE_GROUP_DOC", i18n("Use group documentation on undocumented members"));
    addMessage("TAB_SIZE", i18n("Tab size"));
    addMessage("ENABLED_SECTIONS", i18n("Enable conditional sections"));
    addMessage("GENERATE_TODOLIST", i18n("Generate TODO-list"));
    addMessage("GENERATE_TESTLIST", i18n("Generate Test-list"));
    addMessage("GENERATE_BUGLIST", i18n("Generate Bug-list"));
    addMessage("ALIASES", i18n("Aliases"));
    addMessage("MAX_INITIALIZER_LINES", i18n("Maximum lines shown for initializers"));
    addMessage("OPTIMIZE_OUTPUT_FOR_C", i18n("Optimize output for C"));
    addMessage("SHOW_USED_FILES", i18n("Show used files"));
    addMessage("Messages", i18n("Messages"));
    addMessage("QUIET", i18n("Suppress output"));
    addMessage("WARNINGS", i18n("Show warnings"));
    addMessage("WARN_IF_UNDOCUMENTED", i18n("Warn about undocumented entitites"));
    addMessage("WARN_FORMAT", i18n("Warnings format"));
    addMessage("WARN_LOGFILE", i18n("Write warnings to"));
    addMessage("Input", i18n("Input"));
    addMessage("INPUT", i18n("Input files and directories"));
    addMessage("FILE_PATTERNS", i18n("Input patterns"));
    addMessage("RECURSIVE", i18n("Recurse into subdirectories"));
    addMessage("EXCLUDE", i18n("Exclude from input"));
    addMessage("EXCLUDE_PATTERNS", i18n("Exclude patterns"));
    addMessage("EXAMPLE_PATH", i18n("Path to examples"));
    addMessage("EXAMPLE_PATTERNS", i18n("Example patterns"));
    addMessage("IMAGE_PATH", i18n("Path to images"));
    addMessage("INPUT_FILTER", i18n("Input filter"));
    addMessage("FILTER_SOURCE_FILES", i18n("Filter input files"));
    addMessage("Index", i18n("Index"));
    addMessage("ALPHABETICAL_INDEX", i18n("Generate alphabetical index"));
    addMessage("COLS_IN_ALPHA_INDEX", i18n("Columns in index"));
    addMessage("IGNORE_PREFIX", i18n("Prefix to ignore"));
    addMessage("HTML", i18n("HTML"));
    addMessage("GENERATE_HTML", i18n("Generate HTML"));
    addMessage("HTML_OUTPUT", i18n("HTML output directory"));
    addMessage("HTML_HEADER", i18n("Header file"));
    addMessage("HTML_FOOTER", i18n("Footer file"));
    addMessage("HTML_STYLESHEET", i18n("Stylesheet"));
    addMessage("HTML_ALIGN_MEMBERS", i18n("Align members"));
    addMessage("GENERATE_HTMLHELP", i18n("Generate HTML-help"));
    addMessage("GENERATE_CHI", i18n("Generate .chi file"));
    addMessage("BINARY_TOC", i18n("Generate binary TOC"));
    addMessage("TOC_EXPAND", i18n("Expand TOC"));
    addMessage("DISABLE_INDEX", i18n("Disable index"));
    addMessage("ENUM_VALUES_PER_LINE", i18n("Number of enum values per line"));
    addMessage("GENERATE_TREEVIEW", i18n("Generate treeview"));
    addMessage("TREEVIEW_WIDTH", i18n("Treeview width"));
    addMessage("LaTeX", i18n("LaTeX"));
    addMessage("GENERATE_LATEX", i18n("Generate LaTeX"));
    addMessage("LATEX_OUTPUT", i18n("LaTeX output directory"));
    addMessage("COMPACT_LATEX", i18n("Generate compact output"));
    addMessage("PAPER_TYPE", i18n("Paper type"));
    addMessage("EXTRA_PACKAGES", i18n("Include extra packages"));
    addMessage("LATEX_HEADER", i18n("LaTeX Header file"));
    addMessage("PDF_HYPERLINKS", i18n("Generate PDF hyperlinks"));
    addMessage("USE_PDFLATEX", i18n("Use pdflatex"));
    addMessage("LATEX_BATCHMODE", i18n("Use batch mode"));
    addMessage("RTF", i18n("RTF"));
    addMessage("GENERATE_RTF", i18n("Generate RTF"));
    addMessage("RTF_OUTPUT", i18n("RTF output directory"));
    addMessage("COMPACT_RTF", i18n("Generate compact output"));
    addMessage("RTF_HYPERLINKS", i18n("Generate hyperlinks"));
    addMessage("RTF_STYLESHEET_FILE", i18n("Stylesheet"));
    addMessage("RTF_EXTENSIONS_FILE", i18n("Extensions file"));
    addMessage("Man", i18n("Man"));
    addMessage("GENERATE_MAN", i18n("Generate man pages"));
    addMessage("MAN_OUTPUT", i18n("Man output directory"));
    addMessage("MAN_EXTENSION", i18n("Man extension"));
    addMessage("MAN_LINKS", i18n("Generate links"));
    addMessage("Preprocessor", i18n("Preprocessor"));
    addMessage("ENABLE_PREPROCESSING", i18n("Enable preprocessing"));
    addMessage("MACRO_EXPANSION", i18n("Expand macros"));
    addMessage("EXPAND_ONLY_PREDEF", i18n("Expand only predefined macros"));
    addMessage("SEARCH_INCLUDES", i18n("Search for includes"));
    addMessage("INCLUDE_PATH", i18n("Include path"));
    addMessage("INCLUDE_FILE_PATTERNS", i18n("Include file patterns"));
    addMessage("PREDEFINED", i18n("Predefined macros"));
    addMessage("EXPAND_AS_DEFINED", i18n("Expand as defined"));
    addMessage("External", i18n("External"));
    addMessage("TAGFILES", i18n("Tagfiles"));
    addMessage("GENERATE_TAGFILE", i18n("Generate tagfile"));
    addMessage("ALLEXTERNALS", i18n("List all externals"));
    addMessage("PERL_PATH", i18n("Path to Perl"));
    addMessage("Dot", i18n("Dot"));
    addMessage("HAVE_DOT", i18n("Use dot"));
    addMessage("CLASS_GRAPH", i18n("Generate class graphs"));
    addMessage("COLLABORATION_GRAPH", i18n("Generate collaboration graphs"));
    addMessage("INCLUDE_GRAPH", i18n("Generate include graphs"));
    addMessage("INCLUDED_BY_GRAPH", i18n("Generate included-by graphs"));
    addMessage("GRAPHICAL_HIERARCHY", i18n("Generate graphical hierarchy"));
    addMessage("DOT_PATH", i18n("Path to dot"));
    addMessage("MAX_DOT_GRAPH_WIDTH", i18n("Maximum graph width"));
    addMessage("MAX_DOT_GRAPH_HEIGHT", i18n("Maximum graph height"));
    addMessage("GENERATE_LEGEND", i18n("Generate legend"));
    addMessage("DOT_CLEANUP", i18n("Remove intermediate files"));
    addMessage("Search", i18n("Search"));
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

