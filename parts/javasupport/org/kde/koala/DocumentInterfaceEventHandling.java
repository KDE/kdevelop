package org.kde.koala;
import org.kde.qt.*;


/** Override these methods in subclasses of {@link DocumentInterface} to customise event handling behaviour */
public interface DocumentInterfaceEventHandling {
	void childEvent(QChildEvent arg1);
	void customEvent(QCustomEvent arg1);
	void timerEvent(QTimerEvent arg1);
}

