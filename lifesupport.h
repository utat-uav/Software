#ifndef LIFESUPPORT_H
#define LIFESUPPORT_H
#include <QObject>
#include <QProcess>
#include <QTextBrowser>
#include <QString>

class LifeSupport : public QObject{
public:
    LifeSupport(QProcess* classifier, QTextBrowser* consoleOutput) ;
    QProcess* classifier ;
    QTextBrowser* consoleOutput ;
    QString filePath, imagePath ;
    ~LifeSupport() ;
};

#endif // LIFESUPPORT_H
