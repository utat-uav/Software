#include "lifesupport.h"

LifeSupport::LifeSupport(QProcess *classifier, QTextBrowser *consoleOutput){
    this->classifier=classifier;
    this->consoleOutput=consoleOutput;
}

LifeSupport::~LifeSupport(){
    delete classifier;
    delete consoleOutput;
}

