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

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));

  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

  // maybe there is a file open already
  activePartChanged( partController()->activePart() );
}


AStylePart::~AStylePart()
{
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
  KDevFormatter formatter;

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

	KConfig *config = kapp->config();
	config->setGroup("AStyle");
	int wsCount = config->readNumEntry("FillCount",2);
	if (config->readEntry("Fill", "Tabs") == "Tabs")
	{
		// tabs and wsCount spaces to be a tab
		QString replace;
		for (int i =0;i<wsCount;i++)
			replace+=' ';

		indentWith=indentWith.replace(replace, QChar('\t'));
		indentWith=indentWith.remove(' ');
	} else
	{
		if ( config->readBoolEntry("FillForce",false)){
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


void AStylePart::configWidget(KDialogBase *dlg)
{
	QVBox *vbox = dlg->addVBoxPage(i18n("Formatting"), i18n("Formatting"), BarIcon( info()->icon(), KIcon::SizeMedium));
  AStyleWidget *w = new AStyleWidget(this, vbox, "astyle config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
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

QString AStylePart::formatSource( const QString text, AStyleWidget * widget )
{
	ASStringIterator is(text);
	KDevFormatter * formatter = ( widget ? new KDevFormatter( widget ) : new KDevFormatter );

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
    return formatSource(text, 0);
}

QString AStylePart::indentString( ) const
{
  KDevFormatter formatter;
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

#include "astyle_part.moc"
