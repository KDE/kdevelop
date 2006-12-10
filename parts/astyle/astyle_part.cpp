#include "astyle_part.h"

#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <kprogress.h>
#include <kdevcore.h>
#include <kdevapi.h>
#include <kdevpartcontroller.h>
#include <kdevplugininfo.h>
#include <configwidgetproxy.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlineedit.h>
#include <qregexp.h>

#include "astyle_widget.h"
#include "astyle_adaptor.h"

static const KDevPluginInfo data("kdevastyle");

typedef KDevGenericFactory<AStylePart> AStyleFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevastyle, AStyleFactory( data ) )

AStylePart::AStylePart(QObject *parent, const char *name, const QStringList &)
  : KDevSourceFormatter(&data, parent, name ? name : "AStylePart")
{
  setInstance(AStyleFactory::instance());

  setXMLFile("kdevpart_astyle.rc");

  formatTextAction = new KAction(i18n("&Reformat Source"), 0, this, SLOT(beautifySource()), actionCollection(), "edit_astyle");
  formatTextAction->setEnabled(false);
  formatTextAction->setToolTip(i18n("Reformat source"));
  formatTextAction->setWhatsThis(i18n("<b>Reformat source</b><p>Source reformatting functionality using <b>astyle</b> library. "
                             "Also available in <b>New Class</b> and <b>Subclassing</b> wizards."));

  formatFileAction = new KAction(i18n("&Format files"), 0, this, SLOT(formatFilesSelect()), actionCollection(), "tools_astyle");
  formatFileAction->setEnabled(false);
  formatFileAction->setToolTip(i18n("Format files"));
  formatFileAction->setWhatsThis(i18n("<b>Fomat files</b><p>Formatting functionality using <b>astyle</b> library. "
                             "Also available in <b>New Class</b> and <b>Subclassing</b> wizards."));
  formatFileAction->setEnabled ( true );

  m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("Formatting"), GLOBALDOC_OPTIONS, info()->icon());
    m_configProxy->createProjectConfigPage(i18n("Formatting"), PROJECTDOC_OPTIONS, info()->icon());


  connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase* ,QWidget*,unsigned int)), this, SLOT(insertConfigWidget(const KDialogBase*,QWidget*,unsigned int)));

  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));

  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

  // maybe there is a file open already
  activePartChanged( partController()->activePart() );

  loadGlobal();
  //use the globals first, project level will override later..
  m_project=m_global;

}

void AStylePart::loadGlobal()
{
//   kdDebug(9009) << "Load global"<<endl;
  KConfig *config = kapp->config();
  config->setGroup("AStyle");
  QString options = config->readEntry("Options","BlockBreak=0,BlockBreakAll=0,BlockIfElse=0,Brackets=Break,BracketsCloseHeaders=0,FStyle=UserDefined,Fill=Tabs,FillCount=4,FillEmptyLines=0,FillForce=0,IndentBlocks=0,IndentBrackets=0,IndentCases=0,IndentClasses=1,IndentLabels=1,IndentNamespaces=1,IndentPreprocessors=0,IndentSwitches=1,KeepBlocks=1,KeepStatements=1,MaxStatement=40,MinConditional=-1,PadOperators=0,PadParenthesesIn=1,PadParenthesesOut=1,PadParenthesesUn=1,");


 QStringList pairs = QStringList::split( ",", options);
 QStringList::Iterator it;
 for ( it = pairs.begin(); it != pairs.end(); ++it ) {
	QStringList bits = QStringList::split( "=", (*it) );
	m_global[bits[0]] = bits[1];
 }

//   for (QMap<QString, QVariant>::iterator iter = m_global.begin();iter != m_global.end();iter++)
//         {
//               kdDebug(9009) << "load: " <<iter.key() << "="<< iter.data()  << endl;
// 		}
}

void AStylePart::saveGlobal()
{
	QString options;
	 for (QMap<QString, QVariant>::iterator iter = m_global.begin();iter != m_global.end();iter++)
        {
//               kdDebug(9009) <<"saveGlobal" <<iter.key() << "="<< iter.data()  << endl;
			  options += iter.key();
			  options += "=";
			  options += iter.data().toString();
			  options += ",";
		}
// 		for (QMap<QString, QVariant>::iterator iter = m_project.begin();iter != m_project.end();iter++)
//         {
//               kdDebug(9009) << "project before: "  <<iter.key() << "="<< iter.data()  << endl;
// 		}

  KConfig *config = kapp->config();
  config->setGroup("AStyle");
  config->writeEntry("Options",options);

  config->sync();
//   	 for (QMap<QString, QVariant>::iterator iter = m_global.begin();iter != m_global.end();iter++)
//         {
//               kdDebug(9009) << "global after: "  <<iter.key() << "="<< iter.data()  << endl;
// 		}
// 		for (QMap<QString, QVariant>::iterator iter = m_project.begin();iter != m_project.end();iter++)
//         {
//               kdDebug(9009) << "project after: "  <<iter.key() << "="<< iter.data()  << endl;
// 		}
}

AStylePart::~AStylePart()
{
  saveGlobal();
  delete m_configProxy;
}

void AStylePart::beautifySource()
{
  KTextEditor::EditInterface *iface
      = dynamic_cast<KTextEditor::EditInterface*>(partController()->activePart());
  if (!iface)
    return;

    bool has_selection = false;

  KTextEditor::SelectionInterface *sel_iface
      = dynamic_cast<KTextEditor::SelectionInterface*>(partController()->activePart());
  if (sel_iface && sel_iface->hasSelection())
    has_selection = true;

  //if there is a selection, we only format it.
  ASStringIterator is(has_selection ? sel_iface->selection() : iface->text());
  KDevFormatter formatter(m_project);

  formatter.init(&is);

  QString output;
  QTextStream os(&output, IO_WriteOnly);

  // put the selection back to the same indent level.
  // taking note of the config options.
  unsigned int indentCount=0;
  QString indentWith("");
  if ( has_selection){
  	QString original = sel_iface->selection();
	for (;indentCount<original.length();indentCount++){
		QChar ch = original[indentCount];
		if ( ch.isSpace()){
			if ( ch == QChar('\n') || ch == QChar('\r')){
				indentWith="";
			}
			else{
				indentWith+=original[indentCount];
			}
	  	}
		else{
			break;
		}
	}

	int wsCount = m_project["FillCount"].toInt();
	if (m_project["Fill"].toString() == "Tabs")
	{
		// tabs and wsCount spaces to be a tab
		QString replace;
		for (int i =0;i<wsCount;i++)
			replace+=' ';

		indentWith=indentWith.replace(replace, QChar('\t'));
		indentWith=indentWith.remove(' ');
	} else
	{
		if ( m_project["FillForce"].toBool()){
			//convert tabs to spaces
			QString replace;
			for (int i =0;i<wsCount;i++)
				replace+=' ';

			indentWith=indentWith.replace(QChar('\t'),replace);
		}
	}
  }

  while (formatter.hasMoreLines()){
	  if ( has_selection ){
		  os << indentWith;
	  }
	  os << QString::fromUtf8(formatter.nextLine().c_str()) << endl;
  }

  uint col = 0;
  uint line = 0;

  if(has_selection) //there was a selection, so only change the part of the text related to it
  {
    //remove the final newline character, unless it should be there
    if ( !sel_iface->selection().endsWith( "\n" ) )
      output.setLength(output.length()-1);

    sel_iface->removeSelectedText();
    cursorPos( partController()->activePart(), &col, &line );
    iface->insertText( col, line, output);

    return;
  }

  cursorPos( partController()->activePart(), &col, &line );

  iface->setText( output );

  setCursorPos( partController()->activePart(), col, line );
}

void AStylePart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
	switch (pageNo)
	{
		case GLOBALDOC_OPTIONS:
		{
			AStyleWidget *w = new AStyleWidget(this, true, page, "astyle config widget");
			connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
			break;
		}
		case PROJECTDOC_OPTIONS:
		{
			AStyleWidget *w = new AStyleWidget(this, false, page, "astyle config widget");
			connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
			break;
		}
	}
}

QString AStylePart::getExtensions(){
	QString values;
	int count=0;
	 for (QMap<QString, QString>::iterator iter = m_extensions.begin();iter != m_extensions.end();iter++)
     {
		QString ext = iter.data();
		values.append(ext);
		values.append(" ");
		if ( count++ == 1){
			values.append('\n');
			count=0;
		}
	}
	values += " ";
	kdDebug(9009) << "getExtensions " << values<<endl;
	return values.stripWhiteSpace();
}

void AStylePart::setExtensions ( QString ext )
{
	kdDebug(9009) << "setExtensions " << ext<<endl;
	m_extensions.clear();
	QStringList extensions = QStringList::split ( " ", ext );
	for ( QStringList::Iterator iter = extensions.begin(); iter != extensions.end(); iter++ )
	{
		QString ending=*iter;
		if ( ending.startsWith ( "*" ) )
		{
			if (ending.length() ==1 ){
				// special case.. any file.
				m_extensions.insert(ending, ending);
			}
			else{
				m_extensions.insert( ending.mid( 1 ), ending);
			}
		}
		else
		{
			m_extensions.insert(ending, ending);
		}
	}
}

void AStylePart::activePartChanged ( KParts::Part *part )
{
	bool enabled = false;

	KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*> ( part );

	if ( rw_part )
	{
		KTextEditor::EditInterface *iface = dynamic_cast<KTextEditor::EditInterface*> ( rw_part );

		if ( iface )
		{
			// check for the everything case..
			if ( m_extensions.find ( "*" ) == m_extensions.end() )
			{
				QString extension = rw_part->url().path();
				int pos = extension.findRev ( '.' );
				if ( pos >= 0 )
				{
					extension = extension.mid ( pos );
					enabled = ( m_extensions.find ( extension ) != m_extensions.end() );
				}
			}
			else
			{
				enabled = true;
			}
		}
	}

	formatTextAction->setEnabled ( enabled );
}

QString AStylePart::formatSource( const QString text, AStyleWidget * widget, const QMap<QString, QVariant>& options  )
{
	ASStringIterator is(text);
	KDevFormatter * formatter = ( widget)? new KDevFormatter( widget ) : new KDevFormatter(options);

	formatter->init(&is);

	QString output;
	QTextStream os(&output, IO_WriteOnly);

	while ( formatter->hasMoreLines() )
		os << QString::fromUtf8( formatter->nextLine().c_str() ) << endl;

	delete formatter;

	return output;
}

void AStylePart::cursorPos( KParts::Part *part, uint * line, uint * col )
{
	if (!part || !part->inherits("KTextEditor::Document")) return;

	KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
	if (iface)
	{
		iface->cursorPositionReal( line, col );
	}
}

void AStylePart::setCursorPos( KParts::Part *part, uint line, uint col )
{
	if (!part || !part->inherits("KTextEditor::Document")) return;

	KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
	if (iface)
	{
		iface->setCursorPositionReal( line, col );
	}
}

QString AStylePart::formatSource( const QString text )
{
    return formatSource(text, 0, m_project);
}

QString AStylePart::indentString( ) const
{
  KDevFormatter formatter(m_project);
  return formatter.indentString();
}

void AStylePart::contextMenu(QPopupMenu *popup, const Context *context)
{
	if (context->hasType( Context::EditorContext ))
	{
		popup->insertSeparator();
		int id = popup->insertItem( i18n("Format selection"), this, SLOT(beautifySource()) );
		popup->setWhatsThis(id, i18n("<b>Format</b><p>Formats the current selection, if possible"));
	}
	else if ( context->hasType( Context::FileContext )){
		const FileContext *ctx = static_cast<const FileContext*>(context);
		m_urls = ctx->urls();

		popup->insertSeparator();
		int id = popup->insertItem( i18n("Format files"), this, SLOT(formatFiles()) );
		popup->setWhatsThis(id, i18n("<b>Format files</b><p>Formats selected files if possible"));

	}
}

void AStylePart::restorePartialProjectSession(const QDomElement * el)
{
	kdDebug(9009) << "Load project" << endl;
	QDomElement style = el->namedItem("AStyle").toElement();

	if (style.attribute("FStyle", "GLOBAL") == "GLOBAL")
	{
		m_project = m_global;
		m_project["FStyle"] = "GLOBAL";
	}
	else
	{
		for (QMap<QString, QVariant>::iterator iter = m_global.begin();iter != m_global.end();iter++)
        {
              m_project[iter.key()] = style.attribute(iter.key(),iter.data().toString());
		}
	}
	QDomElement exten = el->namedItem("Extensions").toElement();
	QString ext = exten.attribute("ext");
	if ( ext.isEmpty() || ext.stripWhiteSpace().isEmpty()){
		ext="*.C *.H *.c *.c++ *.cc *.cpp *.cxx *.diff *.h *.h++ *.hh *.hpp *.hxx *.inl *.java *.moc *.patch *.tlh *.xpm";
	}
	setExtensions(ext);
}


void AStylePart::savePartialProjectSession(QDomElement * el)
{
	QDomDocument domDoc = el->ownerDocument();
	if (domDoc.isNull())
		return;

	QDomElement style = domDoc.createElement("AStyle");
	style.setAttribute("FStyle", m_project["FStyle"].toString());
	if (m_project["FStyle"] != "GLOBAL")
	{
		 for (QMap<QString, QVariant>::iterator iter = m_project.begin();iter != m_project.end();iter++)
        {
              style.setAttribute(iter.key(),iter.data().toString());
		}
	}
	QDomElement exten = domDoc.createElement ( "Extensions" );
	QString values;
	for ( QMap<QString, QString>::iterator iter = m_extensions.begin();iter != m_extensions.end();iter++ )
	{
		values += ( iter.data() ) + " ";
	}
	exten.setAttribute ( "ext", values );

	el->appendChild(style);
	el->appendChild(exten);
}

void AStylePart::formatFilesSelect(){
	m_urls.clear();
	QStringList filenames = KFileDialog::getOpenFileNames (  QString::null, getExtensions(),0,"Select files to format" );

	for(QStringList::Iterator it = filenames.begin(); it != filenames.end();it++){
		m_urls << *it;
	}
	formatFiles();
}


/**
 * Format the selected files with the current style.
 */
void AStylePart::formatFiles()
{
	KURL::List::iterator it = m_urls.begin();
	while ( it != m_urls.end() )
	{
		kdDebug ( 9009 ) << "Selected " << ( *it ).pathOrURL() << endl;
		++it;
	}

	uint processed = 0;
	KProgressDialog *prog = new KProgressDialog ( 0, "dialog", i18n ( "Formatting files.." ), "", true );
	prog->setMinimumDuration(100);
	prog->show();
	for ( uint fileCount = 0; fileCount < m_urls.size(); fileCount++ )
	{
		QString fileName = m_urls[fileCount].pathOrURL();

		bool found = false;
		for ( QMap<QString, QString>::Iterator it = m_extensions.begin(); it != m_extensions.end(); ++it )
		{
			QRegExp re ( it.data(), true, true );
			if ( re.search ( fileName ) == 0 && ( uint ) re.matchedLength() == fileName.length() )
			{
				found = true;
				break;
			}
		}

		if ( found )
		{
			QString backup = fileName + "#";
			prog->setLabel ( i18n ( "Processing file: %1" ).arg ( fileName ) );
			prog->progressBar()->setValue ( (fileCount+1 / m_urls.size()-1)*100 );

			QFile fin ( fileName );
			QFile fout ( backup );
			if ( fin.open ( IO_ReadOnly ) )
			{
				if ( fout.open ( IO_WriteOnly ) )
				{
					QString fileContents ( fin.readAll() );
					fin.close();
					QTextStream outstream ( &fout );
					outstream << formatSource ( fileContents );
					fout.close();
					QDir().rename ( backup, fileName );
					processed++;
				}
				else
				{
					KMessageBox::sorry ( 0, i18n ( "Not able to write %1" ).arg ( backup ) );
				}
			}
			else
			{
				KMessageBox::sorry ( 0, i18n ( "Not able to read %1" ).arg ( fileName ) );
			}
		}
	}
	prog->hide();
	delete prog;
	if ( processed != 0 )
	{
		KMessageBox::information ( 0, i18n ( "Processed %1 files ending with extensions %2" ).arg ( processed ).arg(getExtensions().stripWhiteSpace()) );
	}
	m_urls.clear();

}


#include "astyle_part.moc"
