#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QAbstractItemModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void setModel(QAbstractItemModel *m);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
