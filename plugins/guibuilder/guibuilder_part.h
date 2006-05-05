#ifndef GUIBUILDER_PART_H
#define GUIBUILDER_PART_H

#include <QObject>
#include <QHash>
#include <kdevreadwritepart.h>

class QAction;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class KAboutData;
class KAction;

template<class T> class QList;

typedef QHash<KAction*, QAction*> DesignerActionHash;


class GuiBuilderPart: public KDevReadWritePart
{
  Q_OBJECT
public:
  GuiBuilderPart(QWidget* parentWidget, const char* widgetName,
                 QObject *parent, const char *name, const QStringList &args);
  virtual ~GuiBuilderPart();

  virtual void setApiInstance( KDevApi* api );

  static KAboutData* createAboutData();

  void setupActions();

  virtual bool openFile();
  virtual bool saveFile();

  QDesignerFormEditorInterface *designer() const;

private:
  //wrap the actions provided by QDesignerFormWindowManagerInterface in
  //KActions
  KAction* wrapDesignerAction( QAction*, KActionCollection*, const char* );
  void updateDesignerAction( KAction*, QAction* );

private Q_SLOTS:
  void updateDesignerActions();


private:
  QDesignerFormEditorInterface *m_designer;
  QDesignerFormWindowInterface *m_window;
  DesignerActionHash m_designerActions;

};

#endif // GUIBUILDER_PART_H
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
