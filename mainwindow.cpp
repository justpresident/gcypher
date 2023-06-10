#include <mainwindow.h>
#include <ui_mainwindow.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMenu();
    createStatusBar();

    listModel = new QStringListModel(this);
    ui->listView->setModel(listModel);

    connect(&store, SIGNAL(changed(const Store &)), this, SLOT(refresh_data(const Store &)));
    connect(&store, SIGNAL(changed(const Store &)), this, SLOT(save_file(const Store &)));

    refresh_data(store);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete listModel;
    if (cypher != NULL)
        delete cypher;
}

void MainWindow::createStatusBar() {
    rowsLabel = new QLabel(this);
    rowsLabel->setFrameShadow(QFrame::Raised);

    statusBar()->addPermanentWidget(rowsLabel);
    statusBar()->showMessage("New file");

}

void MainWindow::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open"));
    saveAction = fileMenu->addAction(tr("&Save"));
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open_file()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save_file()));

    ui->verticalLayout->setMenuBar(menuBar);
}

QString MainWindow::choose_file(const QFileDialog::FileMode mode) {
    QString file_name;

    if (mode == QFileDialog::ExistingFile) {
        file_name = QFileDialog::getOpenFileName(this);
    } else {
        file_name = QFileDialog::getSaveFileName(this);
    }

    return file_name;
}

QString input_password() {
    bool ok;
    QString password = QInputDialog::getText(NULL,"Please input file password", "Password", QLineEdit::Password,"",&ok);
    if (ok && password != "")
        return password;
    else
        return "";
}

Cypher * MainWindow::mk_file_cypher(QFileDialog::FileMode mode) {
    QString data_file = choose_file(mode);
    Cypher *cypher = NULL;

    if (data_file == "")
        return NULL;

    QString key = input_password();
    if (key == "")
        return NULL;

    cypher = new Cypher(data_file, key.toUtf8().constData());

    return cypher;
}

void MainWindow::update_file_name(const QString action) {
    QString message = action + " - " + cypher->get_file_name();
    QFontMetrics fm(statusBar()->font());
    int availableLength = (statusBar()->size().width() - rowsLabel->size().width())/fm.averageCharWidth();
    if (message.length() > availableLength) {
        message = action + " - ..." + cypher->get_file_name().right(availableLength - action.length() - 6);
    }

    statusBar()->showMessage(message, 1500);

    QString window_title = "Cypher - " + cypher->get_file_name();
    if (window_title.length() > availableLength - 15)
        window_title = "Cypher - ..." + cypher->get_file_name().right(availableLength - 20);
    setWindowTitle(window_title);
}

void MainWindow::open_file() {
    cypher = mk_file_cypher(QFileDialog::ExistingFile);
    if (cypher != NULL) {
        qint8 result = cypher->read_data(store);
        if (result) {
            QString err = "Unknown";
            switch(result) {
            case ERR_CYPHER_BAD_FORMAT:
                err = "Bad file format";
                break;
            case ERR_CYPHER_EMPTY_FILE:
                err = "Empty file";
                break;
            }
            statusBar()->showMessage("Error: "+ err);
            cypher = NULL;
            return;
        }

        ui->nameEdit->setText("");
        refresh_data(store);
        if (store.get_data().empty()) {
            statusBar()->showMessage("Error: wrong key or empty file");
            cypher = NULL;
            return;
        }
        update_file_name("Loaded");
    }
}

bool MainWindow::maybe_save() {
    if (cypher != NULL || listModel->stringList().count() == 0)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(
                this,
                tr("Cypher - exit confirmation"),
                tr("Changes are not saved, would you like to save?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return save_file();
    else if (ret == QMessageBox::Cancel)
        return false;

    return true;
}

bool MainWindow::save_file() {
    return save_file(store);
}

bool MainWindow::save_file(const Store &store) {
    if (cypher == NULL)
        cypher = mk_file_cypher(QFileDialog::AnyFile);

    if (cypher != NULL) {
        cypher->write_data(store);
        update_file_name("Saved");

        return true;
    }
    return false;
}

void MainWindow::refresh_data(const Store &store) {
    QString key = ui->nameEdit->text();
    listModel->setStringList(store.get_keys().filter(key));

    QString value = "";
    if (listModel->stringList().contains(key)) {
        value = store.get(ui->nameEdit->text());

        QModelIndex keyIndex = listModel->index(listModel->stringList().indexOf(key), 0);
        ui->listView->setCurrentIndex(keyIndex);
    }
    ui->valueEdit->document()->setPlainText(value);

    rowsLabel->setText(QString::number(listModel->stringList().count()) + " keys");
}

void MainWindow::on_saveButton_clicked()
{
    if (!ui->nameEdit->text().isEmpty())
        store.put(ui->nameEdit->text(), ui->valueEdit->document()->toPlainText(), QDateTime::currentMSecsSinceEpoch() / 1000);
}

void MainWindow::on_deleteButton_clicked()
{
    int row = ui->listView->currentIndex().row();

    if (row < 0) {
        statusBar()->showMessage("No key selected");
        return;
    }

    QString selected_key = listModel->stringList().at(row);
    store.remove(selected_key);
    ui->nameEdit->setText("");
    ui->valueEdit->document()->setPlainText("");
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QString selected_key = listModel->stringList().at(index.row());
    ui->nameEdit->setText(selected_key);
    ui->valueEdit->document()->setPlainText(store.get(selected_key));
}

void MainWindow::on_nameEdit_textChanged(const QString &arg1 __attribute__((unused)))
{
    refresh_data(store);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybe_save()) {
        event->accept();
    } else {
        event->ignore();
    }
}

