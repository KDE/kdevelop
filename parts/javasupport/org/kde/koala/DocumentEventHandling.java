package org.kde.koala;
import org.kde.qt.*;


/** Override these methods in subclasses of {@link Document} to customise event handling behaviour */
public interface DocumentEventHandling {

/** 
 Convenience method which is called when the Part received a {@link PartActivateEvent} .
 Reimplement this if you don't want to reimplement {@link event} and test for the event yourself
 or even install an event filter.
     
*/
	void partActivateEvent(PartActivateEvent event);

/** 
 Convenience method which is called when the Part received a
 {@link PartSelectEvent} .
 Reimplement this if you don't want to reimplement {@link event} and
 test for the event yourself or even install an event filter.
     
*/
	void partSelectEvent(PartSelectEvent event);
	void childEvent(QChildEvent arg1);
	void customEvent(QCustomEvent event);

/** 
 Reimplemented from Part, so that the window caption is set to
 the current url (decoded) when the part is activated
 This is the usual behaviour in 99% of the apps
 Reimplement if you don't like it - test for event.activated() !

 Technical note : this is done with GUIActivateEvent and not with
 PartActivateEvent because it's handled by the mainwindow
 (which gets the even after the PartActivateEvent events have been sent)
   
*/
	void guiActivateEvent(GUIActivateEvent event);
	void timerEvent(QTimerEvent arg1);
}

