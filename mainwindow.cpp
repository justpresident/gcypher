#include <mainwindow.h>
#include <ui_mainwindow.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMenu();

    listModel = new QStringListModel(this);
    ui->listView->setModel(listModel);

    connect(&store, SIGNAL(changed(const Store &)), this, SLOT(refresh_data(const Store &)));
    connect(&store, SIGNAL(changed(const Store &)), this, SLOT(save_file(const Store &)));

    statusBar()->showMessage("New file");

    set_window_size();
    connect(qApp->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(set_window_size()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete listModel;
    if (cypher != NULL)
        delete cypher;
}

void MainWindow::set_window_size() {
    QString sys_type = QSysInfo::productType();
    QStringList mobiles({"android", "blackberry", "ios", "winphone"});

    if (mobiles.contains(sys_type)) {
        resize(qApp->desktop()->availableGeometry(this).size());
    }
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

QString choose_file(QFileDialog::FileMode mode) {
    QFileDialog dialog(NULL);
    dialog.setFileMode(mode);
    dialog.setViewMode(QFileDialog::List);

    if (dialog.exec()) {
        QStringList fileNames = dialog.selectedFiles();
        return fileNames[0];
    } else
        return "";
}

QString input_password() {
    bool ok;
    QString password = QInputDialog::getText(NULL,"Please input file password", "Password", QLineEdit::Password,"",&ok);
    if (ok && password != "")
        return password;
    else
        return "";
}

Cypher * mk_file_cypher(QFileDialog::FileMode mode) {
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
        refresh_data(store, true);
        if (store.get_data().empty()) {
            statusBar()->showMessage("Error: wrong key or empty file");
            cypher = NULL;
            return;
        }
        set_file_name("Loaded");
    }
}

void MainWindow::save_file() {
    save_file(store);
}

void MainWindow::set_file_name(const QString action) {
    statusBar()->showMessage(action + " - " + cypher->get_file_name(), 1500);
    setWindowTitle("Cypher - " + cypher->get_file_name());
}

void MainWindow::save_file(const Store &store) {
    if (cypher == NULL)
        cypher = mk_file_cypher(QFileDialog::AnyFile);

    if (cypher != NULL) {
        cypher->write_data(store);
        set_file_name("Saved");
    }
}

void MainWindow::refresh_data(const Store &store, bool reset_name_edit = false) {
    QString key = ui->nameEdit->text();
    listModel->setStringList(store.get_keys().filter(key));

    if (reset_name_edit) {
        ui->nameEdit->setText("");
    }

    QString value = "";
    if (listModel->stringList().contains(key)) {
        value = store.get(ui->nameEdit->text());

        QModelIndex keyIndex = listModel->index(listModel->stringList().indexOf(key), 0);
        ui->listView->setCurrentIndex(keyIndex);
    }
    ui->valueEdit->document()->setPlainText(value);
}

void MainWindow::on_saveButton_clicked()
{
    store.put(ui->nameEdit->text(),ui->valueEdit->document()->toPlainText());
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
