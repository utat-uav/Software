#include "interoplogin.h"
#include "ui_interoplogin.h"

#include <QSettings>

InteropLogin::InteropLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InteropLogin)
{
    ui->setupUi(this);

    QSettings settings;
    QString defaultIP = settings.value("Interop/IP", QString("128.0.0.1")).toString();
    int defaultPort = settings.value("Interop/Port", (int)0).toInt();
    QString defaultUser = settings.value("Interop/User", QString("")).toString();

    ui->ipEdit->setText(defaultIP);
    ui->portEdit->setValue(defaultPort);
    ui->userEdit->setText(defaultUser);

//    if (ui->userEdit->text() == "")
//    {
//        ui->userEdit->setFocus();
//    }
//    else
//    {
//        ui->passEdit->setFocus();
//    }
}

InteropLogin::~InteropLogin()
{
    delete ui;
}

void InteropLogin::accept()
{
    QSettings settings;
    settings.setValue("Interop/IP", ui->ipEdit->text());
    settings.setValue("Interop/Port", ui->portEdit->value());
    settings.setValue("Interop/User", ui->userEdit->text());

    serverInfo.ip = ui->ipEdit->text();
    serverInfo.port = ui->portEdit->value();
    serverInfo.username = ui->userEdit->text();
    serverInfo.password = ui->passEdit->text();

    QDialog::accept();
}

void InteropLogin::on_loginButton_clicked()
{
    this->accept();
}

InteropLogin::ServerInfo InteropLogin::getServerInfo()
{
    return this->serverInfo;
}
