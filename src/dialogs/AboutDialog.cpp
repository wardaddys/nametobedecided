/**
 * @file AboutDialog.cpp
 * @brief Implementation of AboutDialog
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("About TunerPro");
    setFixedSize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("TunerPro", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #00BCD4;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    QLabel *versionLabel = new QLabel("Version 1.0.0-alpha", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel);
    
    QLabel *descLabel = new QLabel("Open-Source ECU Tuning Application\nfor Speeduino", this);
    descLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descLabel);
    
    layout->addSpacing(20);
    
    QLabel *creditsLabel = new QLabel("Developed by:\nSafeerullah Afridi (FA-22-151)\nMuhammad Saeed Sajid (FA-22-143)\n\nSupervised by:\nProf. Dr. Shariq Hussain", this);
    creditsLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(creditsLabel);
    
    layout->addStretch();
    
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &AboutDialog::accept);
    layout->addWidget(closeButton);
}
