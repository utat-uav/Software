#include "loadingbardialog.h"
#include "ui_loadingbardialog.h"

#include <mutex>

LoadingBarDialog::LoadingBarDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingBarDialog)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Tool);
}

LoadingBarDialog::~LoadingBarDialog()
{
    delete ui;
}

void LoadingBarDialog::setStatus(QString status)
{
    ui->statusLabel->setText(status);
}

static std::mutex loadingLock;
void LoadingBarDialog::setPercent(int percent)
{
    loadingLock.lock();
    ui->progressBar->setValue(percent);
    loadingLock.unlock();
}
