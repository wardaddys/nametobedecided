/**
 * @file ConnectionDialog.cpp
 * @brief Implementation of ConnectionDialog
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"
#include "../utils/Settings.h"

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
    
    // Connect refresh button
    connect(ui->refreshButton, &QPushButton::clicked, this, &ConnectionDialog::refreshPorts);
    
    // Populate baud rates
    ui->baudCombo->addItem("9600", 9600);
    ui->baudCombo->addItem("19200", 19200);
    ui->baudCombo->addItem("38400", 38400);
    ui->baudCombo->addItem("57600", 57600);
    ui->baudCombo->addItem("115200", 115200);
    
    // Set default baud rate from settings
    int lastBaud = Settings::getLastBaudRate();
    int index = ui->baudCombo->findData(lastBaud);
    if (index != -1) {
        ui->baudCombo->setCurrentIndex(index);
    } else {
        ui->baudCombo->setCurrentIndex(4); // Default to 115200
    }
    
    refreshPorts();
}

ConnectionDialog::~ConnectionDialog() {
    delete ui;
}

void ConnectionDialog::refreshPorts() {
    ui->portCombo->clear();
    
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        ui->portCombo->addItem(port.portName() + " (" + port.description() + ")", port.portName());
    }
    
    // Select last used port
    QString lastPort = Settings::getLastPort();
    int index = ui->portCombo->findData(lastPort);
    if (index != -1) {
        ui->portCombo->setCurrentIndex(index);
    }
}

QString ConnectionDialog::getPortName() const {
    return ui->portCombo->currentData().toString();
}

int ConnectionDialog::getBaudRate() const {
    return ui->baudCombo->currentData().toInt();
}
