#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagewidget.h"
#include "loadingbardialog.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    loading(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Read window settings
    QSettings window_settings;
    restoreGeometry(window_settings.value("WindowGeometry", geometry()).toByteArray());
    restoreState(window_settings.value("WindowState").toByteArray());

    // Initializes variables
    cellWidth = 0;
    rowCount = 0;
    items = new QList<ImageWidget*>;
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(0);
    noTabs = true ;

    // Sets number of columns
    setColumnCount(5);

    // Get application path
    QString exePath = QDir::currentPath();
    ui->consoleOutput->append("Current path: " + exePath + "\n");

    // Get potential files for classifier exe
    QList<QString> files;
    listFiles(exePath, "", files);

    QString classifierPath;
    QString cnnPath;

    // Find classifier exe
    for (int i = 0; i < files.size(); ++i)
    {
        if (files[i].contains("SoftmaxRegression.exe"))
        {
            classifierPath = files[i];
            cnnPath = classifierPath;
            cnnPath.replace("SoftmaxRegression.exe", "TrainedCNN");
            break;
        }
    }

    // Get args
    QStringList args;
    cnnPath = cnnPath.replace('/', '\\');
    args << "-cnn" << cnnPath;

    // Start classifier
    classifier = new QProcess(this);
    classifier->start(classifierPath, args, QProcess::Unbuffered | QProcess::ReadWrite);
    classifier->waitForStarted();

    // Connect slots for piping the standard output
    connect(classifier, SIGNAL(readyReadStandardOutput()), this, SLOT(ReadOut()));
    connect(classifier, SIGNAL(readyReadStandardError()), this, SLOT(ReadErr()));

    // Set up completer
    completer = new QCompleter(prevCommands, this);
    ui->consoleCommander->setCompleter(completer);
    dataPackage = new LifeSupport(classifier,ui->consoleOutput);

    loadingBarDialog = new LoadingBarDialog(this);
}

void MainWindow::ReadOut()
{
    QProcess *p = dynamic_cast<QProcess *>(sender());
    if (p)
    {
        ui->consoleOutput->moveCursor(QTextCursor::End);
        ui->consoleOutput->insertPlainText(p->readAllStandardOutput());
        ui->consoleOutput->moveCursor(QTextCursor::End);
    }
}

void MainWindow::ReadErr()
{
    QProcess *p = dynamic_cast<QProcess *>(sender());
    if (p)
    {
        //ui->consoleOutput->moveCursor(QTextCursor::End);
        //ui->consoleOutput->insertPlainText("ERROR: " + p->readAllStandardError());
        //ui->consoleOutput->moveCursor(QTextCursor::End);
    }
}

void MainWindow::listFiles(QDir directory, QString indent, QList<QString> &files)
{
    indent += "\t";
    QDir dir(directory);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo finfo, list) {
        files.append(finfo.filePath());
        if(finfo.isDir()) {
            listFiles(QDir(finfo.absoluteFilePath()), indent, files);
        }
    }
}

void MainWindow::setColumnCount(int col) {
    colCount = col;
    ui->photoListTable->setColumnCount(colCount);
}

MainWindow::~MainWindow()
{
    QSettings window_settings;
    window_settings.setValue("WindowGeometry", saveGeometry());
    window_settings.setValue("WindowState", saveState());

    classifier->close();
    classifier->terminate();

    foreach (const auto &item, *items)
    {
        delete item;
    }

    delete items;
    delete ui;
    delete classifier;
    delete completer;
    delete loadingBarDialog;
}

void MainWindow::on_MainWindow_destroyed()
{

}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    resizeTable();
    QWidget::resizeEvent(e);
}

void MainWindow::resizeTable()
{
    //return; // Disable resizing

    // Gets table width
    tableWidth = ui->photoListTable->width()-19;

    // Calculates cell width based on table width and number of columns
    cellWidth = tableWidth/colCount;

    // Calculates cell height based on cell width
    cellHeight = cellWidth * 4/5 + 23;

    // Rethinks the number of columns
    /*
    if (tableWidth < 1150) {
        if (colCount != 4) {
            setColumnCount(4);
            refreshTable();
        }
    }
    else {
        if (colCount != 5) {
            setColumnCount(5);
            refreshTable();
        }
    }
    */

    // Resizes each column
    for (int i = 0; i < colCount; i++) {
        ui->photoListTable->setColumnWidth(i, cellWidth);
    }

    // Resizes rows
    ui->photoListTable->verticalHeader()->setDefaultSectionSize(cellHeight);
}

void MainWindow::refreshTable()
{
    qDebug() << "Begin table refresh";
    resizeTable();

    // Makes copy of the items
    QList<ImageWidget *> *itemsCopy = new QList<ImageWidget *>;
    for (int i = 0; i < items->size(); i++) {
        ImageWidget *temp = new ImageWidget(dataPackage, this, false);
        // Copy all information over
        temp->setTitle(items->at(i)->getTitle());
        temp->setFilePath(items->at(i)->getFilePath());
        temp->setFolderPath(items->at(i)->getFolderPath());
        temp->setImagePath(items->at(i)->getImagePath());
        temp->setImage(items->at(i)->getImage());
        temp->setNumTargets(items->at(i)->getNumTargets());
        temp->setSeen(items->at(i)->getSeen());

        // Preserve the old targetList window
        temp->deleteTargetListWindow();
        temp->changeTargetListWindow(items->at(i)->getTargetList(), items->at(i)->isInitialized());
        items->at(i)->changeTargetListWindow(NULL);

        itemsCopy->append(temp);
        (items->at(i))->finishLoading();
        delete &*(items->at(i));
    }

    // Clears table
    ui->photoListTable->clear();

    // Recalculates rowCount
    rowCount = ceil((double) items->size() / (double) colCount);
    ui->photoListTable->setRowCount(rowCount);

    // Calculate max col number on the last row
    int cMax = (itemsCopy->size() - (rowCount-1)*colCount) % (colCount+1);

    // Sets new table contents
    int itemCount = 0;
    for (int r = 0; r < rowCount; r++) {

        for (int c = 0; c < colCount; c++) {
            if (!(r == rowCount-1 && c == cMax)) { // ends if the end of the list is hit
                items->replace(itemCount, itemsCopy->at(itemCount));

                // Sets the widget
                ui->photoListTable->setCellWidget(r, c, items->at(itemCount));

                itemCount++;
            }
            else{
                break;
            }
        }

        // Disables extra spaces
        if (r == rowCount-1) {
            for (int c = cMax; c < colCount; c++) {
                ui->photoListTable->setItem(r, c, new QTableWidgetItem());
                ui->photoListTable->item(r, c)->setFlags(Qt::ItemIsSelectable);
            }
        }
    }

    // Take care of memory
    delete itemsCopy;

    qDebug() << "Done refreshing";
}

QList<ImageWidget*>* MainWindow::getItems()
{
    return this->items;
}

void MainWindow::appendItem(QString folderPath, QString filePath, QString imagePath, QString title, int numTargets)
{
    // Creates item
    ImageWidget *newWidget = new ImageWidget(dataPackage, this, false);
    newWidget->setTitle(title);
    newWidget->setImage(imagePath);
    newWidget->setFilePath(filePath);
    newWidget->setFolderPath(folderPath);
    newWidget->setNumTargets(numTargets);
    items->append(newWidget);
}

void MainWindow::indexToCoordinates(int index, int *r, int *c)
{
    // Row index
    *r = ceil((double) (index+1) / (double) colCount) - 1;

    // Column index
    *c = ((index+1) - *r*colCount) % (colCount+1) - 1;
}

void MainWindow::on_loadButton_clicked()
{
    if (loading) return;

    QSettings config(QDir::currentPath()+"config.ini",QSettings::IniFormat);
    // Gets the directory from a separate window
    QString dir = QFileDialog::getExistingDirectory(this, tr("Load Directory..."), config.value("User Settings/Image Folder","/home").toString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    config.setValue("User Settings/Image Folder",dir);

    if (dir == "") return;

    // Get mumber of files
    QStringList nameFilter;
    nameFilter.append("*.ini");
    QDir directory(dir);
    QStringList fileList = directory.entryList(nameFilter);

    loadingBarDialog->setWindowTitle("Loading");
    loadingBarDialog->setStatus("Loading images...");
    loadingBarDialog->setPercent(0);
    loadingBarDialog->show();

    // Create and connect thread
    MainWindowLoader* loader = new MainWindowLoader(this, dir);
    connect(loader, &MainWindowLoader::finished, loader, &QObject::deleteLater);
    connect(loader, &MainWindowLoader::destroyed, this, [=](){
        loadingBarDialog->hide();
        refreshTable();
        loading = false;
    });
    connect(loader, &MainWindowLoader::statusUpdate, this, [=](int value){
        if (loadingBarDialog)
        {
            loadingBarDialog->setPercent((double)value/fileList.size()*100.0);
        }
    });
    loading = true;

    // Start thread
    loader->start();
}

void MainWindowLoader::run()
{
    // Create filter
    QStringList nameFilter;
    nameFilter.append("*.ini");

    // Scans through directory, applying the filter
    QDir directory(dir);
    QStringList fileList = directory.entryList(nameFilter);

    qDebug() << "HI";

    // Goes through each file and opening the image
    int count = 0;
    foreach (QString file, fileList)
    {
        QString filePath = dir+"/"+file ;
        QSettings resultFile(filePath,QSettings::IniFormat);
        QString imagePath = resultFile.value("Analysis Parameters/IMAGE","").toString() ; //pass directory to image widget
        int numTargets = resultFile.value("Crop Info/Number of Crops", "").toInt(); // gets the number of targets
        QFileInfo fileInfo(imagePath);
        QString filename(fileInfo.fileName());

        if (imagePath != "")
        {
            mainWindow->appendItem(dir, filePath, imagePath, filename, numTargets);
            ++count;
            if (count%5 == 0)
            {
                emit statusUpdate(count);
            }
        }
    }
}

void MainWindow::on_addItemButton_clicked()
{
    //addItem("");
}

void MainWindow::on_editButton_clicked()
{
    QItemSelectionModel *select = ui->photoListTable->selectionModel();
    QModelIndexList selected = select->selectedIndexes();

    if (select->hasSelection() && selected.length() == 1) {
        // Gets the selected index
        QList<QModelIndex>::iterator i = selected.begin();
        int selectedIndex = (i->row())*colCount + i->column();

        // Makes an edit dialog
        ItemMaker *editDialog = new ItemMaker();
        editDialog->setModal(true);
        editDialog->setWindowTitle("Edit");

        // Sets initial information
        editDialog->setTitle(items->at(selectedIndex)->getTitle());
        editDialog->setFilePath(items->at(selectedIndex)->getImagePath());

        // Starts the dialog
        editDialog->exec();

        // If okay was pressed in the edit dialog
        if (editDialog->getAccepted()) {
            // Gets information from edit dialog
            QString title = editDialog->getTitle();
            QString filePath = editDialog->getFilePath();

            // Sets item information
            items->at(selectedIndex)->setTitle(title);
            items->at(selectedIndex)->setImage(filePath);
        }

        delete editDialog;
    }
}

void MainWindow::on_deleteItemButton_clicked()
{
    QItemSelectionModel *select = ui->photoListTable->selectionModel();
    QModelIndexList selected = select->selectedIndexes();
    QList<int> deletionOrder;

    if (select->hasSelection()) {
        // Makes sure everything is deleted in the correct order
        for (QList<QModelIndex>::iterator i = selected.begin(); i != selected.end(); i++) {
            int index = (i->row()) * colCount + i->column();
            deletionOrder.append(index);
        }
        // Sorts into descending order
        qSort(deletionOrder.begin(), deletionOrder.end(), qGreater<int>());

        // Deletes items in the table
        for (int i = 0; i < deletionOrder.length(); i++) {
            int index = deletionOrder.at(i);
            items->removeAt(index);
        }

        // Clears selection
        ui->photoListTable->selectionModel()->clearSelection();

        refreshTable();
    }
}

void MainWindow::addTab(QWidget* newTab, QString title) {
    if (noTabs)
        ui->tabWidget->removeTab(0);

    ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(newTab, title));
    noTabs = false ;
}

// Returns false if not found
bool MainWindow::findTab(QWidget *tab){
    int index = ui->tabWidget->indexOf(tab);
    if (index == -1) return false;
    ui->tabWidget->setCurrentIndex(index);
    return true;
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    TargetListWindow* targetListWindow = (TargetListWindow*)ui->tabWidget->widget(index);
    ui->tabWidget->removeTab(index);
    ((ImageWidget*)targetListWindow->parent)->changeTargetListWindow(NULL, false);
    delete targetListWindow;
    if (ui->tabWidget->count()==0){
        noTabs = true ;
        //ui->tabWidget->addTab(new TargetListWindow, "Target List") ;
    }
}

void MainWindow::on_consoleCommander_returnPressed()
{
    // Get command
    QString command = ui->consoleCommander->text();
    QStringList::iterator it = std::find(prevCommands.begin(), prevCommands.end(), command);
    if (it == prevCommands.end())
        prevCommands << command;
    command += "\n";

    // Update autocompleter
    QStringListModel *model = (QStringListModel*)(completer->model());
    if(model == NULL)
        model = new QStringListModel();
    model->setStringList(prevCommands);
    completer->setModel(model);

    // Send command
    classifier->write(command.toStdString().c_str());

    // Clear commander
    ui->consoleCommander->setText("");

}

void MainWindow::on_actionProcess_Image_Set_triggered()
{
    ImageSetProcessor *processor = new ImageSetProcessor();
    processor->setModal(true);
    processor->setWindowTitle("Image Processor");

    // Starts the dialog
    processor->exec();

    delete processor;
}
