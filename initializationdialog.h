#ifndef INITIALIZATIONDIALOG_H
#define INITIALIZATIONDIALOG_H

#include <QDialog>

namespace Ui {
class InitializationDialog;
}

class InitializationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitializationDialog(QWidget *parent = 0);
    ~InitializationDialog();

private:
    Ui::InitializationDialog *ui;
};

#endif // INITIALIZATIONDIALOG_H
