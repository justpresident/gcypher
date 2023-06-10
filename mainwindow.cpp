#include <ui_mainwindow.h>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    createMenu();
    createStatusBar();

    keyList_ = new QStringListModel(this);
    ui_->listView->setModel(keyList_);

    connect(&store_, SIGNAL(changed(const Store &)), this, SLOT(refresh_data(const Store &)));
    connect(&store_, SIGNAL(changed(const Store &)), this, SLOT(save_file(const Store &)));

    refresh_data(store_);
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete keyList_;
    if (cypher_ != NULL)
        delete cypher_;
}

void MainWindow::createStatusBar() {
    rowsLabel_ = new QLabel(this);
    rowsLabel_->setFrameShadow(QFrame::Raised);

    statusBar()->addPermanentWidget(rowsLabel_);
    statusBar()->showMessage("New file");

}

void MainWindow::createMenu()
{
    menuBar_ = new QMenuBar;

    fileMenu_ = new QMenu(tr("&File"), this);
    openAction_ = fileMenu_->addAction(tr("&Open"));
    saveAction_ = fileMenu_->addAction(tr("&Save"));
    exitAction_ = fileMenu_->addAction(tr("E&xit"));
    menuBar_->addMenu(fileMenu_);

    connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));
    connect(openAction_, SIGNAL(triggered()), this, SLOT(open_file()));
    connect(saveAction_, SIGNAL(triggered()), this, SLOT(save_file()));

    ui_->verticalLayout->setMenuBar(menuBar_);
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
    Cypher *result = NULL;

    if (data_file == "")
        return NULL;

    QString key = input_password();
    if (key == "")
        return NULL;

    result = new Cypher(data_file, key.toUtf8().constData());

    return result;
}

void MainWindow::update_file_name(const QString action) {
    QString message = action + " - " + cypher_->get_file_name();
    QFontMetrics fm(statusBar()->font());
    int availableLength = (statusBar()->size().width() - rowsLabel_->size().width())/fm.averageCharWidth();
    if (message.length() > availableLength) {
        message = action + " - ..." + cypher_->get_file_name().right(availableLength - action.length() - 6);
    }

    statusBar()->showMessage(message, 1500);

    QString window_title = "Cypher - " + cypher_->get_file_name();
    if (window_title.length() > availableLength - 15)
        window_title = "Cypher - ..." + cypher_->get_file_name().right(availableLength - 20);
    setWindowTitle(window_title);
}

void MainWindow::open_file() {
    cypher_ = mk_file_cypher(QFileDialog::ExistingFile);
    if (cypher_ != NULL) {
        qint8 result = cypher_->read_data(store_);
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
            cypher_ = NULL;
            return;
        }

        ui_->nameEdit->setText("");
        refresh_data(store_);
        if (store_.get_data().empty()) {
            statusBar()->showMessage("Error: wrong key or empty file");
            cypher_ = NULL;
            return;
        }
        update_file_name("Loaded");
    }
}

bool MainWindow::maybe_save() {
    if (cypher_ != NULL || keyList_->stringList().count() == 0)
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
    return save_file(store_);
}

bool MainWindow::save_file(const Store &store) {
    if (cypher_ == NULL)
        cypher_ = mk_file_cypher(QFileDialog::AnyFile);

    if (cypher_ != NULL) {
        cypher_->write_data(store);
        update_file_name("Saved");

        return true;
    }
    return false;
}

void MainWindow::refresh_data(const Store &store) {
    QString key = ui_->nameEdit->text();
    keyList_->setStringList(store.get_keys().filter(key));

    QString value = "";
    if (keyList_->stringList().contains(key)) {
        value = store.get(ui_->nameEdit->text());

        QModelIndex keyIndex = keyList_->index(keyList_->stringList().indexOf(key), 0);
        ui_->listView->setCurrentIndex(keyIndex);
    }
    ui_->valueEdit->document()->setPlainText(value);

    rowsLabel_->setText(QString::number(keyList_->stringList().count()) + " keys");
}

void MainWindow::on_saveButton_clicked()
{
    if (!ui_->nameEdit->text().isEmpty())
        store_.put(ui_->nameEdit->text(), ui_->valueEdit->document()->toPlainText(), QDateTime::currentMSecsSinceEpoch() / 1000);
}

void MainWindow::on_deleteButton_clicked()
{
    int row = ui_->listView->currentIndex().row();

    if (row < 0) {
        statusBar()->showMessage("No key selected");
        return;
    }

    QString selected_key = keyList_->stringList().at(row);
    store_.remove(selected_key);
    ui_->nameEdit->setText("");
    ui_->valueEdit->document()->setPlainText("");
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QString selected_key = keyList_->stringList().at(index.row());
    ui_->nameEdit->setText(selected_key);
    ui_->valueEdit->document()->setPlainText(store_.get(selected_key));
}

void MainWindow::on_nameEdit_textChanged(const QString &arg1 __attribute__((unused)))
{
    refresh_data(store_);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybe_save()) {
        event->accept();
    } else {
        event->ignore();
    }
}

