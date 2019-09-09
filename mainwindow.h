#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QList>
#include <QMessageBox>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void addQComboBox();
    bool initSerialPort();
    void sendMsg(const QString &msg);
    void Sleep(int msec);

public slots:
    void recvMsg();
    void openSerialPort();
    void clearWindows();
    void onNewPortList(QStringList portName);
    void comboxChangeHandle(int);

private:
    Ui::MainWindow *ui;

    QSerialPort *serialPort;
};

#endif // MAINWINDOW_H
