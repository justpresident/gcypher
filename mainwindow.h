#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QInputDialog>
#include <QStatusBar>

#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>

#include <cypher.h>
#include <store.h>

class QMenu;
class QMenuBar;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void refresh_data(const Store &store);
    void open_file();
    void save_file();
    void save_file(const Store &store);
    void set_file_name(const QString action);

private slots:
    void on_saveButton_clicked();

    void on_deleteButton_clicked();

    void on_listView_clicked(const QModelIndex &index);

    void on_nameEdit_textChanged(const QString &arg1);

    void on_clearButton_clicked();

private:
    Ui::MainWindow *ui;
    Cypher *cypher = NULL;
    Store store;
    void createMenu();

    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *exitAction, *openAction, *saveAction;

    QStringListModel *listModel;
};

#endif // MAINWINDOW_H
