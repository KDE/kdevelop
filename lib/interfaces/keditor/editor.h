#ifndef __EDITOR_H__
#define __EDITOR_H__


#include <sys/stat.h>
#include <time.h>

#include <qstring.h>


#include <dcopref.h>
#include <kxmlguiclient.h>
#include <kparts/part.h>
#include <kurl.h>
class KAction;


#include "interface.h"


namespace KEditor {
        

class Editor;
class DocumentDCOPIface;


/**
 * \brief The basic interface for a document
 *
 * The basic interface for a document
 * contained in the editor.
 *
 * The basic interface allows to load and save the file.
 *
 * A document can be queried for more complex interfaces.
 */

class Document : public KParts::ReadWritePart
{
  Q_OBJECT

public:
 
  /**
   * \brief Query for an advanced interface
   *
   * Returns a pointer to a more
   * advanced document interface.
   *
   * \param ifname The name of the interface
   *
   * \return Pointer to the interface, or 0 if the document
   * does not provide that interface.
   */ 
  DocumentInterface *queryInterface(const QString &ifname);


  /**
   * \brief Has file been modified on disc?
   *
   * Tells if the file has been modified on the disc while
   * being edited.
   *
   * \return true, if the file has been modified,
   *         false, else
   */
  bool modifiedOnDisc();
  

  /// return a reference to the DCOP interface
  DCOPRef dcopInterface() const;


protected:  
  
  Document(Editor *parent);

  Editor *parent() const { return _parent; };


  /// resets the time of last modification
  void resetModifiedTime();

  /// Should the file be saved?
  bool shouldBeSaved();
 
  
protected slots:

  void slotSaveAs();
  void slotSave();
  void slotRevert();


private:

  Editor *_parent;

  time_t _mtime;
 
  DocumentDCOPIface *m_dcopIface;

};


/**
 * \brief The main interface to the editor.
 *
 * This is the main class used to access an editor.
 *
 * An Editor is a KPart, so you can use all the KPart methods
 * to locate and use it.
 *
 * To locate the preferred editor service, use code like this:
 *
 * \code
 *   KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);
 *   if (offers.isEmpty())
 *     return ;
 *        
 *   KService *service = *offers.begin();
 * \endcode
 *
 * Now that we have a reference to the preferred editor service, we
 * load the library to get the factory:
 *
 * \code 
 *   KLibFactory *factory = KLibLoader::self()->factory(service->library());
 * \endcode
 *
 * Now that we have the factory, we create an instance of the editor:
 *
 * \code
 *   Editor *m_editor = static_cast < Editor* > (factory->create(this, "editortest_part", "KParts::ReadWritePart" ));
 * \endcode
 *
 * Now all that is left to do is to activate the part. Inside a KParts::MainWindow, this is done with:
 * \code
 *   if (m_editor)
 *   {
 *     setCentralWidget(m_editor->widget());
 *     createGUI(m_editor);
 *   }
 * \endcode
 */

class Editor : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

  friend class Document;

public:

  /**
   * \brief Create an editor part.
   *
   * Create an instance of the Editor part.
   *
   * \param parent The parent part.
   * \param name The name for the part.
   */
  Editor(QObject *parent=0, const char *name=0);

  /**
   * \brief Deallocate the part's ressources.
   *
   * Deallocates the part's ressources.
   */
  ~Editor();

  /**
   * \brief Query for an advanced interface
   *
   * Query the Editor part to find out if a particular
   * interface is supported.
   *
   * You use it like this:
   *
   * \code
   *   ClipboardEditorIface *iface = static_cast<ClipboardEditorIface*>(getInterface("ClipboardEditorIface"));
   *   if (iface)
   *     ...
   * \endcode
   *
   * Note that implementations of the Editor part are free to implement
   * a subset of the available interfaces, so users of the part should
   * always test if a given interface is provided.
   *
   */
  EditorInterface *queryInterface(const QString &ifname);

  virtual Document *document(const KURL &url) = 0;
  virtual Document *createDocument(QWidget *parentWidget=0, const KURL &url="") = 0;
  virtual Document *currentDocument();


signals:

  void documentActivated(KEditor::Document *document);
  

public slots:

  void activePartChanged(KParts::Part *part);


private:

  Document *_currentDocument;

};

}


#endif
