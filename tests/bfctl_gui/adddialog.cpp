#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDialog)
{
    ui->setupUi(this);
    ui->leSourceHostIP->setEnabled(false);

}

AddDialog::~AddDialog()
{
    delete ui;
}

void AddDialog::rbAnyone()
{
    ui->leSourceHostIP->setEnabled(false);
}
void AddDialog::rdIPHost()
{
    ui->leSourceHostIP->setEnabled(true);
}
