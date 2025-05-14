#ifndef QTMATERIALCOMBOBOX_P_H
#define QTMATERIALCOMBOBOX_P_H

#include "qtmaterialtextfield_p.h"

class QWidget;
class QVBoxLayout;
class QtMaterialComboBoxOverlay;
class QtMaterialComboBoxStateMachine;

class QtMaterialComboBoxPrivate : public QtMaterialTextFieldPrivate
{
    Q_DISABLE_COPY(QtMaterialComboBoxPrivate)
    Q_DECLARE_PUBLIC(QtMaterialComboBox)

public:
    QtMaterialComboBoxPrivate(QtMaterialComboBox *q);
    virtual ~QtMaterialComboBoxPrivate();

    void init();

    QWidget                            *menu;
    QWidget                            *frame;
    QtMaterialComboBoxStateMachine *stateMachine;
    QVBoxLayout                        *menuLayout;
    QStringList                         dataSource;
    int                                 maxWidth;
};

#endif // QTMATERIALCOMBOBOX_P_H
