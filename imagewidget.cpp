#include "imagewidget.h"
#include "ui_imagewidget.h"

ImageWidget::ImageWidget(LifeSupport *dataPackage, MainWindow *parent, bool initTargetList) :
    QWidget(parent),
    ui(new Ui::ImageWidget)
{
    imagePath = "";
    title = "";
    ui->setupUi(this);

    this->seen = false;
    mainWindow = parent ;
    this->dataPackage=dataPackage;

    // Initialize imageLabel
    //setImage(":/images/Untitled.png");

    //ui->imageLabel->setScaledContents(true);
    //ui->imageLabel->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    if (initTargetList)
        targetList = new TargetListWindow(dataPackage);
    else
        targetList = NULL;
    targetListInitialized = false;


    ui->colourLabel->setStyleSheet("QLabel { background-color : yellow; color : blue; }");
}

ImageWidget::~ImageWidget()
{
    delete targetList;
    delete ui;
}

QString ImageWidget::getTitle() const
{
    return this->title;
}

int ImageWidget::getNumTargets() const
{
    return this->numTargets;
}

QString ImageWidget::getImagePath() const
{
    return this->imagePath;
}

QPixmap& ImageWidget::getImage()
{
    return this->image;
}

QString ImageWidget::getFolderPath() const
{
    return this->folderPath;
}

QString ImageWidget::getFilePath() const
{
    return this->filePath;
}

bool ImageWidget::getSeen() const
{
    return this->seen;
}

void ImageWidget::setTitle(QString name)
{
    ui->imageCaption->setText(name);
    title = name;
}

void ImageWidget::setSeen(bool seen)
{
    if (seen)
        ui->colourLabel->setStyleSheet("QLabel { background-color : green; color : blue; }");
    else
        ui->colourLabel->setStyleSheet("QLabel { background-color : yellow; color : blue; }");
    this->seen = seen;
}

void ImageWidget::setFolderPath(QString folderPath){
    this->folderPath = folderPath ;
}

void ImageWidget::setFilePath(QString filePath){
    this->filePath = filePath ;
}

void ImageWidget::setImagePath(QString imagePath)
{
    this->imagePath = imagePath;
}

void ImageWidget::setNumTargets(int numTargets)
{
    QString s = QString::number(numTargets);
    QString numTargetsString = s + " targets";
    this->ui->numTargetDisplay->setText(numTargetsString);
    this->numTargets = numTargets;
}

void ImageWidget::setImage(QString imagePath)
{
    if (imagePath != "") {
        QPixmap *pix = new QPixmap();
        pix->load(imagePath);
        image = pix->scaled(220, 220, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
        ui->imageLabel->setPixmap(image);
        this->imagePath = imagePath;
        delete pix;
    }
}

void ImageWidget::setImage(QPixmap &resizedImage)
{
    ui->imageLabel->setPixmap(resizedImage);
    image = resizedImage;
}

bool ImageWidget::isInitialized() const
{
    return this->targetListInitialized;
}

void ImageWidget::deleteTargetListWindow()
{
    delete this->targetList;
    this->targetList = NULL;
}

/*bool ImageWidget::getSeen() const
{
    return this->seen;
}*/

void ImageWidget::changeTargetListWindow(TargetListWindow* targetList, bool alreadyInitialized)
{
    this->targetList = targetList;
    this->targetListInitialized = alreadyInitialized;
}

TargetListWindow* ImageWidget::getTargetList() const
{
    return this->targetList;
}

/*void ImageWidget::setTabWidget(QTabWidget* tabWidget ) {
    this->tabWidget = tabWidget ;
}*/

void ImageWidget::mouseDoubleClickEvent(QMouseEvent *event){
    if ( event->button() == Qt::LeftButton ){
        ui->colourLabel->setStyleSheet("QLabel { background-color : green; color : blue; }"); // Mark as seen
        this->seen = true;

        dataPackage->filePath=filePath;

        /*
        if (!targetListInitialized) {
            targetList->setWindowFlags(Qt::Window);
            targetList->setModal(false);
            targetList->setWindowTitle(title);
            targetList->setMainPic(imagePath);
            targetList->loadTargets(folderPath, filePath) ;
            targetListInitialized = true;
        }
        targetList->show();*/

        if (!targetListInitialized) {
            //TargetListWindow* newTab = new TargetListWindow ;
            targetList->setMainPic(imagePath);
            targetList->loadTargets(folderPath, filePath) ;
            mainWindow->addTab(targetList, title) ;
            targetListInitialized = true;
        }
        else {
            bool found = mainWindow->findTab(targetList) ;
            if (!found)
            {
                mainWindow->addTab(targetList, title);
            }
        }
    }
}

void ImageWidget::on_pinButton_clicked()
{
    QList<ImageWidget*>* itemsList = mainWindow->getItems();

    for (int i = 0; i < itemsList->size(); ++i)
    {
        if (itemsList->at(i) == this) {
            itemsList->removeAt(i);
            break;
        }
    }

    itemsList->insert(0, this);

    mainWindow->refreshTable();
}
