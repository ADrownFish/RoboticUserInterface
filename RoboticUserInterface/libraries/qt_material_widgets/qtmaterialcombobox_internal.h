#ifndef QTMATERIALCOMBOBOXSTATEMACHINE_H
#define QTMATERIALCOMBOBOXSTATEMACHINE_H

#include <QStateMachine>
#include "qtmaterialcombobox.h"

class QtMaterialComboBoxStateMachine : public QStateMachine
{
    Q_OBJECT

public:
    explicit QtMaterialComboBoxStateMachine(QWidget *menu);
    ~QtMaterialComboBoxStateMachine();

signals:
    void shouldOpen();
    void shouldClose();
    void shouldFade();

private:
    Q_DISABLE_COPY(QtMaterialComboBoxStateMachine)

    QWidget *const m_menu;
    QState  *const m_closedState;
    QState  *const m_openState;
    QState  *const m_closingState;
};

#endif // QTMATERIALCOMBOBOXSTATEMACHINE_H
