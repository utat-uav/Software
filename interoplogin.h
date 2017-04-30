#ifndef INTEROPLOGIN_H
#define INTEROPLOGIN_H

#include <QDialog>
#include <QNetworkCookie>

namespace Ui {
class InteropLogin;
}

class InteropLogin : public QDialog
{
    Q_OBJECT

public:

    struct ServerInfo
    {
        QString ip;
        int port;
        QString username;
        QString password;

        QNetworkCookie cookie;
        bool loggedIn = false;
    };

    explicit InteropLogin(QWidget *parent = 0);
    ~InteropLogin();

    void accept();

    ServerInfo getServerInfo();

private slots:
    void on_loginButton_clicked();

private:
    Ui::InteropLogin *ui;
    ServerInfo serverInfo;
};

#endif // INTEROPLOGIN_H
