#ifndef QTMATERIALCOMBOBOX_H
#define QTMATERIALCOMBOBOX_H

#include "qtmaterialtextfield.h"

class QtMaterialComboBoxPrivate;

class QtMaterialComboBox : public QtMaterialTextField
{
    Q_OBJECT

public:
    explicit QtMaterialComboBox(QWidget *parent = 0);
    ~QtMaterialComboBox();

    void setDataSource(const QStringList &data);

signals:
    void itemSelected(QString);

protected slots:
    void updateResults(QString text);

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(QtMaterialComboBox)
    Q_DECLARE_PRIVATE(QtMaterialComboBox)
};

#endif // QTMATERIALCOMBOBOX_H
