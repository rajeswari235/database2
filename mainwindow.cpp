#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial=new QSerialPort(this);
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()){
        ui->comboBox->addItem(info.portName());

    }
    connect(ui->comboBox,SIGNAL(activated(const QString &)),this,SLOT(onPortChanged(const QString &)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow:: onPortChanged(const QString &portName)
{
 if(serial->isOpen()){
    serial->close();
 }
 serial->setPortName(portName);
 serial->setBaudRate(QSerialPort::Baud9600);
 serial->setDataBits(QSerialPort::Data8);
 serial->setParity(QSerialPort::NoParity);
 serial->setStopBits(QSerialPort::OneStop);
 serial->setFlowControl(QSerialPort::NoFlowControl);

 if (serial->open(QIODevice::ReadWrite)) {
     QString msg = QString("Serial port %1 opened successfully").arg(portName);
    QMessageBox::information(this,"Success",msg);
 } else {
     QMessageBox::warning(this,"Error","Serial Port is not open");
}
}
void MainWindow::readData() {
    qDebug()<<"data received";
    buffer.append(serial->readAll());
    responseTimer->start();  // restart timer on every byte received
}

void MainWindow::onResponseTimeout() {
    if (buffer.isEmpty()) return;

    QStringList hexList;
    for (char byte : buffer) {
        hexList.append(QString("%1").arg(static_cast<quint8>(byte), 2, 16, QLatin1Char('0')).toUpper());
    }

    // Save to CSV
    saveDataToCSV(hexList);

    // Display in table
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    for (int col = 0; col < hexList.size(); ++col) {
        ui->tableWidget->setItem(row, col, new QTableWidgetItem(hexList[col]));
    }

    buffer.clear();  // clear for next packet
}

void MainWindow::saveDataToCSV(const QStringList &hexList) {
    QFile file("log_data.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << hexList.join(",") << "\n";
        file.close();
    }
}

