#ifndef GUIBUILDER_PART_H
#define GUIBUILDER_PART_H

#include <QObject>
#include <QHash>
#include <QtCore/QPointer>
#include <kparts/part.h>
#include <kdevdocumentcontroller.h>

class QAction;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWorkspace;

class KAboutData;
class KAction;

template<class T> class QList;

typedef QHash<KAction*, QAction*> DesignerActionHash;


class GuiBuilderPart: public KParts::ReadWritePart
{
  Q_OBJECT
public:
  GuiBuilderPart(QWidget* parentWidget, QObject *parent, const QStringList &args);
  virtual ~GuiBuilderPart();

  static KAboutData* createAboutData();

  void setupActions();

  virtual bool openFile();
  virtual bool saveFile();

  QDesignerFormEditorInterface *designer() const;

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  //wrap the actions provided by QDesignerFormWindowManagerInterface in
  //KActions
  KAction* wrapDesignerAction( QAction*, KActionCollection*, const char* );
  void updateDesignerAction( KAction*, QAction* );

private Q_SLOTS:
  void updateDesignerActions();
  void activated( KDevDocument* );


private:
  QPointer<QDesignerFormEditorInterface> m_designer;
  QPointer<QDesignerFormWindowInterface> m_window;
  QPointer<QWorkspace> m_workspace;

  DesignerActionHash m_designerActions;

};

#endif // GUIBUILDER_PART_H
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
