#include "kdevcomponent.h"

KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Part(parent, name){
}

KDevComponent::~KDevComponent(){
}

void KDevComponent::registerManager(KDevComponentManager* kdcm){
	m_pKdevComponentManager = kdcm;
}

KDevComponentManager* KDevComponent::componentManager(){
	return m_pKdevComponentManager;
}

void KDevComponent::setupGUI(){
}

void KDevComponent::setupComponent(){
}

void KDevComponent::slotConfigWidgetRequested(KDialogBase*){
}

void KDevComponent::slotStopButtonClicked(){
}

void KDevComponent::slotProjectSpaceOpened(){
}

void KDevComponent::slotProjectSpaceClosed(){
}

void KDevComponent::setWidget(QWidget*){
}

QWidget *KDevComponent::widget(){
  return 0;
}

KAboutData* KDevComponent::aboutPlugin(){
  return 0;
}

QList<KAction> KDevComponent::kdevNodeActions(KDevNode*){
  return QList<KAction>();
}

#include "kdevcomponent.moc"
