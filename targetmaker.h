#ifndef TARGETMAKER_H
#define TARGETMAKER_H

#include <QDialog>

namespace Ui {
class TargetMaker;
}

class TargetMaker : public QDialog
{
    Q_OBJECT

public:
    explicit TargetMaker(QWidget *parent = 0);
    ~TargetMaker();

    // Getters
    QString getName() const;
    QString getDesc() const;
    QString getCoord() const;
    QString getImageFilePath() const;
    bool getAccepted() const;

    // Setters
    void setDefaultNameInput(QString defaultNameInput);
    void setDefaultCoordInput(QString defaultCoordInput);
    void setDefaultDescInput(QString defaultDescInput);
    void setDefaultFileInput(QString defaultFileInput);
    void setDefaultInputs();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_toolButton_clicked();

private:
    Ui::TargetMaker *ui;

    bool accepted; // = false;
    QString defaultNameInput; // = "";
    QString defaultCoordInput; // = "";
    QString defaultDescInput; // = "";
    QString defaultFileInput; // = "";
};

#endif // TARGETMAKER_H
