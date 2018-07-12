#include "initializationdialog.h"
#include "ui_initializationdialog.h"

InitializationDialog::InitializationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitializationDialog)
{
    ui->setupUi(this);
}

InitializationDialog::~InitializationDialog()
{
    delete ui;
}
