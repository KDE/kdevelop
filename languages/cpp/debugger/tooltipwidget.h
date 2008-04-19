
#ifndef TOOLTIP_H_f2dc78af9cdf452ce712f673b2bbcd00
#define TOOLTIP_H_f2dc78af9cdf452ce712f673b2bbcd00

#include "util/activetooltip.h"
#include "util/treeview.h"
#include "mi/gdbmi.h"

#include <QPoint>

class QItemSelectionModel;
class QString;
class QResizeEvent;

namespace GDBDebugger
{
    class TreeModel;
    class TreeItem;
    class GDBController;
    class Variable;

    class VariableToolTip : public ActiveToolTip
    {
        Q_OBJECT
        public:
        VariableToolTip(QWidget* parent, QPoint position, 
                        GDBController* controller,
                        const QString& identifier);

        void handleCreated(const GDBMI::ResultRecord& r);

        void addWatch(const GDBMI::ResultRecord& r);

        void addWatchpoint(const GDBMI::ResultRecord& r);

    private slots:
        void slotLinkActivated(const QString& link);
        void slotRangeChanged(int min, int max);

    private:
        TreeModel* model_;
        Variable* var_;
        QItemSelectionModel* selection_;
        GDBController* controller_;
        int itemHeight_;
        AsyncTreeView* view_;
    };
}

#endif
