#include "adddialog.h"
#include "ui_adddialog.h"

#include <QHostAddress>
#include <QMessageBox>

AddDialog::AddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDialog)
{
    ui->setupUi(this);
    ui->leSourceHostIP->setEnabled(false);
    ui->leSourcePort->setValidator(new QIntValidator(0,65535));

    // QRegExp rx("\\b(([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\b");

    QString Octet = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    ui->leSourceHostIP->setValidator(new QRegExpValidator(
             QRegExp("^" + Octet + "\\." + Octet + "\\." + Octet + "\\." + Octet + "$"), this));

    connect(ui->rbAnyone,SIGNAL(toggled(bool)),SLOT(rbAnyone_toggled(bool)));
    connect(ui->rbHostname,SIGNAL(toggled(bool)),SLOT(rbHostname_toggled(bool)));

    QStringList sl;
    sl << "UDP" << "TCP";
    ui->cbProto->addItems(sl);
}

AddDialog::~AddDialog()
{
    delete ui;
}

void AddDialog::rbAnyone_toggled(bool)
{
    ui->leSourceHostIP->setEnabled(false);
}

void AddDialog::rbHostname_toggled(bool)
{
    ui->leSourceHostIP->setEnabled(true);

}

void AddDialog::setChain(bf_chain_t chain)
{
     mFr.base.chain = chain;

}

void AddDialog::accept()
{
    mFr.base.proto = get_proto(ui->cbProto->currentText().toStdString().c_str());

    mFr.base.src_port = ui->leSourcePort->text().toUInt();

    bool any = ui->rbAnyone->isChecked();

    QHostAddress hostaddr(any?"0.0.0.0":ui->leSourceHostIP->text());

    if( hostaddr.isNull() )
    {
        /*QMessageBox::StandardButton reply =*/ QMessageBox::critical(this, tr("Data error")
                             , tr("Wrong ip address")
                             , QMessageBox::Ok );

    }
    else
    {
        mFr.base.s_addr.addr = static_cast<quint32>(htonl(hostaddr.toIPv4Address()));
        emit accept_rule(mFr);
        QDialog::accept();
    }

}

void AddDialog::reject()
{

    QDialog::reject();
}
