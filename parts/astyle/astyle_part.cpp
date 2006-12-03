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

#include <kdevcore.h>
#include <kdevapi.h>
#include <kdevpartcontroller.h>
#include <kdevplugininfo.h>
#include <configwidgetproxy.h>
#include <kapplication.h>
#include <kconfig.h>


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

  _action = new KAction(i18n("&Reformat Source"), 0,
			  this, SLOT(beautifySource()), actionCollection(), "edit_astyle");
  _action->setEnabled(false);
  _action->setToolTip(i18n("Reformat source"));
  _action->setWhatsThis(i18n("<b>Reformat source</b><p>Source reformatting functionality using <b>astyle</b> library. "
                             "Also available in <b>New Class</b> and <b>Subclassing</b> wizards."));

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
  QString options = config->readEntry("Options","BlockBreak=0,BlockBreakAll=0,BlockIfElse=0,Brackets=Break,BracketsCloseHeaders=0,FStyle=UserDefined,Fill=Tabs,FillCount=4,FillEmptyLines=0,FillForce=0,IndentBlocks=0,IndentBrackets=0,IndentCases=1,IndentClasses=1,IndentLabels=1,IndentNamespaces=1,IndentPreprocessors=0,IndentSwitches=0,KeepBlocks=0,KeepStatements=0,MaxStatement=40,MinConditional=-1,PadOperators=1,PadParenthesesIn=1,PadParenthesesOut=0,PadParenthesesUn=1,");

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


void AStylePart::activePartChanged(KParts::Part *part)
{
  bool enabled = false;

  KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(part);

  if (rw_part)
  {
    KTextEditor::EditInterface *iface = dynamic_cast<KTextEditor::EditInterface*>(rw_part);

    if (iface)
    {
      QString extension = rw_part->url().path();

      int pos = extension.findRev('.');
      if (pos >= 0)
        extension = extension.mid(pos);
      if (extension == ".h"   || extension == ".c" || extension == ".java"
       || extension == ".cpp" || extension == ".hpp"
       || extension == ".C"   || extension == ".H"
       || extension == ".cxx" || extension == ".hxx"
       || extension == ".inl" || extension == ".tlh"
       || extension == ".moc" || extension == ".xpm"
       || extension == ".diff"|| extension == ".patch"
       || extension == ".hh"  || extension == ".cc"
       || extension == ".c++" || extension == ".h++")
	enabled = true;
    }
  }

  _action->setEnabled(enabled);
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
	if (!context->hasType( Context::EditorContext ))
		return;

	popup->insertSeparator();
	int id = popup->insertItem( i18n("Format selection"), this, SLOT(beautifySource()) );
	popup->setWhatsThis(id, i18n("<b>Format</b><p>Formats the current selection, if possible"));
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

	el->appendChild(style);
}


#include "astyle_part.moc"
