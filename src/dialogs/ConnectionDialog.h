/**
 * @file ConnectionDialog.h
 * @brief Dialog for selecting serial port and connection settings
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QSerialPortInfo>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();
    
    QString getPortName() const;
    int getBaudRate() const;
    
private slots:
    void refreshPorts();
    
private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
