#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>

#include <QString>
#include <QStringList>
#include <QStringListModel>

#include <QSysInfo>

#include <QDateTime>

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
    QString choose_file(const QFileDialog::FileMode mode);
    void open_file();
    bool save_file();
    bool save_file(const Store &store);
    void update_file_name(const QString action);

    bool maybe_save();
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_saveButton_clicked();

    void on_deleteButton_clicked();

    void on_listView_clicked(const QModelIndex &index);

    void on_nameEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui_;
    Cypher *cypher_ = NULL;
    Store store_;
    void createMenu();
    void createStatusBar();
    Cypher * mk_file_cypher(QFileDialog::FileMode mode);

    QMenuBar *menuBar_;
    QMenu *fileMenu_;
    QAction *exitAction_, *openAction_, *saveAction_;

    QLabel *rowsLabel_;

    QStringListModel *keyList_;
};

#endif // MAINWINDOW_H
