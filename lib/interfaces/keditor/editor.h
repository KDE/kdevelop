#ifndef __EDITOR_H__
#define __EDITOR_H__


#include <qstring.h>


#include <kparts/part.h>
class KAction;


#include "interface.h"


namespace KEditor {
		

class Editor;

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

class Document : public QObject
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
  DocumentInterface *getInterface(QString ifname);

  /**
   * \brief Load file.
   * 
   * Load the contents of a file into the document
   *
   * This method is \e abstract. It has to be implemented
   * by the actual document.
   *
   * \param filename The name of the file to load.
   *
   * \return true, if file was loaded, false if not
   */
  virtual bool load(QString filename) = 0;

  /**
   * \brief Save file as.
   *
   * Save the contents of the document under a given filename.
   *
   * This method is \e abstract. It has to be implemented
   * by the actual document.
   * 
   * \param filename The name of the file to save.
   *
   * \return true, if the file was saved, false if not
   */
  virtual bool save(QString filename) = 0;


  /** 
   * \brief Save file.
   *
   * Save the content of the document under it's current name.
   *
   * \return true, if the file was saved, false if not.
   */
  virtual bool save();
 
protected: 
  /**
   * \brief Change the name.
   *
   * Change the name of the document. This is the name
   * that will be used in the \a save method.
   */
  virtual void rename(QString filename);
  
public:

  /**
   * \brief The current filename.
   *
   * The current name of the document.
   *
   * \return The name.
   */
  virtual QString fileName() const;

  /**
   * \brief A short name.
   *
   * A shortened name of the file, e.g. for editor tabs.
   *
   * \return S short version of the filename.
   */
  virtual QString shortName() const;


signals:

  /**
   * \bried File name has changed.
   *
   * Emitted whenever the name of the document changes.
   *
   * \param name The new name of the doument.
   */
  void fileNameChanged(QString name);


protected:  
  
  Document(Editor *parent);

  Editor *parent() { return _parent; };


private:

  Editor *_parent;

  QString _fileName;
  
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

class Editor : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  /**
   * \brief Create an editor part.
   *
   * Create an instance of the Editor part.
   *
   * \param parentWidget The parent widget for the part.
   * \param widgetName The name for the part widget.
   * \param parent The parent part.
   * \param name The name for the part.
   */
  Editor(QWidget *parentWidget, const char *widgetName, QObject *parent=0, const char *name=0);

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
   * Here are some of the most common interfaces:
   *   - ClipboardEditorIface Access to the clipboard (cut, copy, paste)
   *   - UndoEditorIface Access to the Undo/Redo mechanism
   */
  EditorInterface *getInterface(QString ifname);

  virtual Document *getDocument(const QString &filename=QString::null) = 0;
  virtual void closeDocument(Document *doc) = 0;
  virtual Document *currentDocument() = 0;
  

signals:

  void documentActivated(Document *doc);
  void documentAdded();
  void documentRemoved();


protected:

  virtual bool openFile();


private slots:
		
  void slotLoadFile();
  void slotSaveFile();
  void slotSaveFileAs();
  void slotNewFile();
  void slotCloseFile();

  void documentCountChanged();

			   
private:
			   
  KAction *_openAction, *_saveAction, *_saveAsAction, *_newAction, *_closeAction;

};

}


#endif
