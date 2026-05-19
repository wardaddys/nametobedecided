#include "ProjectWizardOverlay.h"
#include "../core/SerialManager.h"
#include "../utils/Logger.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QDir>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include "../core/ECUDefinition.h"

ProjectWizardOverlay::ProjectWizardOverlay(QWidget *parent)
    : QWidget(parent), m_downloader(new FileDownloader(this)), m_serialManager(nullptr) {
    hide();
    setupUi();
}

void ProjectWizardOverlay::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    m_panel = new QWidget(this);
    m_panel->setFixedSize(550, 480);
    m_panel->setObjectName("WizardPanel");
    m_panel->setStyleSheet(
        "#WizardPanel {"
        "  background-color: #121A2F;"
        "  border: 1px solid #3D5070;"
        "  border-radius: 8px;"
        "}"
        "QLabel {"
        "  color: #EEF2FF;"
        "  font-family: 'DM Sans', sans-serif;"
        "  font-size: 13px;"
        "}"
        "QLineEdit, QTextEdit, QComboBox {"
        "  background-color: #0B101A;"
        "  border: 1px solid #3D5070;"
        "  color: #EEF2FF;"
        "  padding: 6px;"
        "  border-radius: 4px;"
        "}"
        "QComboBox::drop-down { border: none; }"
        "QPushButton {"
        "  background-color: #1A2640;"
        "  color: #EEF2FF;"
        "  border: 1px solid #3D5070;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #2A3C5C; }"
        "QPushButton#DetectBtn {"
        "  background-color: #f57f17;"
        "  color: white;"
        "  border: none;"
        "}"
        "QPushButton#DetectBtn:hover { background-color: #fbc02d; }"
    );

    QVBoxLayout *panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(16);

    // Title
    QLabel *titleLabel = new QLabel("Create New Project", m_panel);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #00E5C8;");
    panelLayout->addWidget(titleLabel);

    m_stack = new QStackedWidget(m_panel);
    panelLayout->addWidget(m_stack);

    // --- STEP 1: Details ---
    QWidget *step1 = new QWidget();
    QVBoxLayout *l1 = new QVBoxLayout(step1);
    l1->setContentsMargins(0,0,0,0);
    
    l1->addWidget(new QLabel("Project Name"));
    m_nameEdit = new QLineEdit();
    l1->addWidget(m_nameEdit);

    l1->addWidget(new QLabel("Project Directory"));
    QHBoxLayout *dirLayout = new QHBoxLayout();
    m_dirEdit = new QLineEdit();
    m_dirEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/OSTunerProjects/MyCar");
    QPushButton *browseBtn = new QPushButton("Browse");
    connect(browseBtn, &QPushButton::clicked, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", m_dirEdit->text());
        if (!dir.isEmpty()) {
            m_dirEdit->setText(dir);
        }
    });
    dirLayout->addWidget(m_dirEdit);
    dirLayout->addWidget(browseBtn);
    l1->addLayout(dirLayout);

    l1->addWidget(new QLabel("Firmware (ECU Definition)"));
    QHBoxLayout *fwLayout = new QHBoxLayout();
    m_firmwareEdit = new QLineEdit();
    m_firmwareEdit->setReadOnly(true);
    m_firmwareEdit->setPlaceholderText("Select an INI file or Detect...");
    
    m_browseIniBtn = new QPushButton("Browse...");
    connect(m_browseIniBtn, &QPushButton::clicked, this, [this](){
        QString file = QFileDialog::getOpenFileName(this, "Select ECU Definition (INI)", "", "INI Files (*.ini)");
        if (!file.isEmpty()) {
            m_selectedIniPath = file;
            m_firmwareEdit->setText(QFileInfo(file).fileName());
            updateButtons();
        }
    });

    m_detectBtn = new QPushButton("Detect");
    m_detectBtn->setObjectName("DetectBtn");
    connect(m_detectBtn, &QPushButton::clicked, this, &ProjectWizardOverlay::detectFirmware);
    
    fwLayout->addWidget(m_firmwareEdit);
    fwLayout->addWidget(m_browseIniBtn);
    fwLayout->addWidget(m_detectBtn);
    l1->addLayout(fwLayout);

    l1->addWidget(new QLabel("Project Description"));
    m_descEdit = new QTextEdit();
    l1->addWidget(m_descEdit);

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #FF5252; font-style: italic;");
    l1->addWidget(m_statusLabel);
    
    m_stack->addWidget(step1);

    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](){ updateButtons(); });
    connect(m_dirEdit, &QLineEdit::textChanged, this, [this](){ updateButtons(); });

    // --- STEP 2: Configuration ---
    m_step2Widget = new QWidget();
    QVBoxLayout *l2 = new QVBoxLayout(m_step2Widget);
    l2->setContentsMargins(0,0,0,0);
    l2->addStretch();
    m_stack->addWidget(m_step2Widget);

    // --- STEP 3: Communications ---
    QWidget *step3 = new QWidget();
    QVBoxLayout *l3 = new QVBoxLayout(step3);
    l3->setContentsMargins(0,0,0,0);
    
    l3->addWidget(new QLabel("Primary COM Port"));
    m_comPortCombo = new QComboBox();
    l3->addWidget(m_comPortCombo);
    
    l3->addWidget(new QLabel("Baud Rate"));
    m_baudCombo = new QComboBox();
    m_baudCombo->addItems({"115200", "57600", "38400", "19200", "9600"});
    m_baudCombo->setCurrentText("115200");
    l3->addWidget(m_baudCombo);

    l3->addStretch();
    m_stack->addWidget(step3);

    // --- Navigation ---
    QHBoxLayout *navLayout = new QHBoxLayout();
    m_cancelBtn = new QPushButton("Cancel");
    m_backBtn = new QPushButton("< Back");
    m_nextBtn = new QPushButton("Next >");
    
    connect(m_cancelBtn, &QPushButton::clicked, this, &QWidget::hide);
    connect(m_backBtn, &QPushButton::clicked, this, &ProjectWizardOverlay::prevStep);
    connect(m_nextBtn, &QPushButton::clicked, this, [this](){
        if (m_stack->currentIndex() == m_stack->count() - 1) {
            finishWizard();
        } else {
            nextStep();
        }
    });

    navLayout->addWidget(m_cancelBtn);
    navLayout->addStretch();
    navLayout->addWidget(m_backBtn);
    navLayout->addWidget(m_nextBtn);

    panelLayout->addLayout(navLayout);
    mainLayout->addWidget(m_panel);
    
    updateButtons();
}

void ProjectWizardOverlay::startWizard() {
    m_stack->setCurrentIndex(0);
    m_nameEdit->clear();
    m_firmwareEdit->clear();
    m_descEdit->clear();
    m_selectedIniPath.clear();
    
    m_comPortCombo->clear();
    for (const QSerialPortInfo& portInfo : QSerialPortInfo::availablePorts()) {
        m_comPortCombo->addItem(portInfo.portName());
    }
    m_comPortCombo->addItem("SIMULATOR");
    
    updateButtons();
    show();
    raise();
}

void ProjectWizardOverlay::nextStep() {
    int nextIdx = m_stack->currentIndex() + 1;
    
    if (m_stack->currentIndex() == 0 && nextIdx == 1) {
        buildDynamicConfigStep();
    }
    
    if (nextIdx < m_stack->count()) {
        m_stack->setCurrentIndex(nextIdx);
        updateButtons();
    }
}

void ProjectWizardOverlay::prevStep() {
    int prevIdx = m_stack->currentIndex() - 1;
    if (prevIdx >= 0) {
        m_stack->setCurrentIndex(prevIdx);
        updateButtons();
    }
}

void ProjectWizardOverlay::buildDynamicConfigStep() {
    QVBoxLayout *l2 = qobject_cast<QVBoxLayout*>(m_step2Widget->layout());
    if (!l2) return;

    // Clear existing dynamic layout items
    QLayoutItem *child;
    while ((child = l2->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    m_dynamicCombos.clear();

    if (m_selectedIniPath.isEmpty()) {
        l2->addWidget(new QLabel("No ECU Definition selected."));
        l2->addStretch();
        return;
    }

    ECUDefinition def;
    if (!def.load(m_selectedIniPath)) {
        l2->addWidget(new QLabel("Failed to load selected ECU Definition."));
        l2->addStretch();
        return;
    }

    auto groups = def.getSettingGroups();
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        const ECUDefinition::SettingGroup &group = it.value();
        
        QLabel *label = new QLabel(group.displayName);
        l2->addWidget(label);
        
        QComboBox *combo = new QComboBox();
        for (int i = 0; i < group.options.size(); ++i) {
            const auto &opt = group.options[i];
            QString text = opt.label;
            if (opt.name == "DEFAULT") text += " (Default)";
            combo->addItem(text, opt.name);
        }
        
        m_dynamicCombos.insert(group.referenceName, combo);
        l2->addWidget(combo);
    }
    
    l2->addStretch();
}

void ProjectWizardOverlay::updateButtons() {
    bool canGoNext = true;
    QString status;

    if (m_stack->currentIndex() == 0) {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            canGoNext = false;
            status = "Please enter a project name.";
        } else if (m_selectedIniPath.isEmpty()) {
            canGoNext = false;
            status = "ECU Definition (INI) is required. Click Detect or Browse.";
        }
    }
    
    if (m_statusLabel) m_statusLabel->setText(status);

    m_backBtn->setEnabled(m_stack->currentIndex() > 0);
    m_nextBtn->setEnabled(canGoNext);
    
    if (m_stack->currentIndex() == m_stack->count() - 1) {
        m_nextBtn->setText("Finish");
        m_nextBtn->setStyleSheet("background-color: #00E5C8; color: #121A2F; font-weight: bold;");
    } else {
        m_nextBtn->setText("Next >");
        m_nextBtn->setStyleSheet("");
    }
}

void ProjectWizardOverlay::finishWizard() {
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) name = "MyProject";
    
    QString path = m_dirEdit->text();
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create TunerStudio compatible folder structure
    dir.mkpath("projectCfg");
    dir.mkpath("inc");
    dir.mkpath("DataLogs");
    dir.mkpath("dashboard");
    dir.mkpath("TuneView");
    
    // Copy the selected INI file
    if (!m_selectedIniPath.isEmpty() && QFile::exists(m_selectedIniPath)) {
        QString dest = dir.absoluteFilePath("projectCfg/mainController.ini");
        if (QFile::exists(dest)) {
            QFile::remove(dest);
        }
        QFile::copy(m_selectedIniPath, dest);
    }
    
    // Create the project.properties file with the user's config
    QFile propsFile(dir.absoluteFilePath("projectCfg/project.properties"));
    if (propsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&propsFile);
        out << "projectName=" << name << "\n";
        out << "portName=" << m_comPortCombo->currentText() << "\n";
        out << "baudRate=" << m_baudCombo->currentText() << "\n";
        
        for (auto it = m_dynamicCombos.begin(); it != m_dynamicCombos.end(); ++it) {
            QString selectedOpt = it.value()->currentData().toString();
            if (!selectedOpt.isEmpty() && selectedOpt != "DEFAULT") {
                out << selectedOpt << "=true\n";
            }
        }
        
        propsFile.close();
    }
    
    emit projectCreated(name, path);
    hide();
}

void ProjectWizardOverlay::detectFirmware() {
    m_detectBtn->setText("Scanning...");
    m_detectBtn->setEnabled(false);
    
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        m_firmwareEdit->setText("No COM ports found. Try 'Browse...' instead.");
        m_detectBtn->setText("Detect");
        m_detectBtn->setEnabled(true);
        return;
    }
    
    QString detectedSig = "";
    QString activePort = "";
    
    // Iterate through available ports and probe for an ECU
    for (const QSerialPortInfo& portInfo : ports) {
        QSerialPort port;
        port.setPort(portInfo);
        port.setBaudRate(115200);
        
        if (port.open(QIODevice::ReadWrite)) {
            // Speeduino reboots on DTR assert. We must wait for the bootloader to finish.
            QEventLoop loop;
            QTimer::singleShot(1500, &loop, &QEventLoop::quit);
            loop.exec();
            
            // Clear bootloader spam
            port.readAll();
            
            // Send 'S' command (Request Signature)
            port.write("S");
            port.waitForBytesWritten(100);
            
            QByteArray response;
            QElapsedTimer timer;
            timer.start();
            
            // Wait up to 1000ms for a response
            while (timer.elapsed() < 1000) {
                if (port.waitForReadyRead(100)) {
                    response.append(port.readAll());
                    // Typically starts with 'S' followed by "speeduino..."
                    if (response.size() > 10) {
                        break;
                    }
                }
            }
            port.close();
            
            // Clean up the response
            if (response.startsWith('S') || response.startsWith('Q')) {
                response = response.mid(1);
            }
            
            QString sigStr = QString::fromUtf8(response).trimmed();
            // Valid Speeduino signatures usually contain the word "speeduino"
            if (!sigStr.isEmpty()) {
                detectedSig = sigStr;
                activePort = portInfo.portName();
                break;
            }
        }
    }
    
    if (!detectedSig.isEmpty()) {
        m_firmwareEdit->setText(detectedSig + " (" + activePort + ")");
        
        // Try to find a matching INI file automatically
        QString matchingIni = findMatchingIni(detectedSig);
        if (!matchingIni.isEmpty()) {
            m_selectedIniPath = matchingIni;
            m_firmwareEdit->setText(detectedSig + " [MATCHED]");
            m_firmwareEdit->setToolTip(matchingIni);
        } else if (detectedSig.toLower().startsWith("speeduino")) {
            if (QMessageBox::question(this, "Definition Missing", 
                QString("No local definition found for '%1'.\n\nWould you like to download it from speeduino.com?").arg(detectedSig),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) 
            {
                m_detectBtn->setText("Downloading...");
                m_detectBtn->setEnabled(false);
                
                QString ver = detectedSig.toLower().replace("speeduino", "").trimmed();
                QString url = "https://speeduino.com/fw/" + ver + ".ini";
                
                QDir().mkpath("C:/OS Tuner/definitions");
                QString savePath = "C:/OS Tuner/definitions/speeduino_" + ver + ".ini";
                
                m_downloader->download(url, savePath, [this](bool success, QString error) {
                    m_detectBtn->setText("Detect");
                    m_detectBtn->setEnabled(true);
                    
                    if (success) {
                        QString text = m_firmwareEdit->text();
                        QString sig = text.split(" (").first();
                        
                        QString matchingIni = findMatchingIni(sig);
                        if (!matchingIni.isEmpty()) {
                            m_selectedIniPath = matchingIni;
                            m_firmwareEdit->setText(sig + " [DOWNLOADED]");
                            m_firmwareEdit->setToolTip(matchingIni);
                            QMessageBox::information(this, "Success", "Definition downloaded and matched successfully!");
                        } else {
                            QMessageBox::warning(this, "Error", "Download finished but matching still failed. Please browse for the file manually.");
                        }
                    } else {
                        QMessageBox::critical(this, "Download Failed", "Failed to download definition:\n" + error);
                    }
                    updateButtons();
                });
                return; // Wait for lambda
            }
        }
    } else {
        m_firmwareEdit->setText("No ECU signature found. Try 'Browse...' instead.");
    }
    
    m_detectBtn->setText("Detect");
    m_detectBtn->setEnabled(true);
    updateButtons();
}

QString ProjectWizardOverlay::findMatchingIni(const QString& signature) {
    QStringList searchPaths;
    searchPaths << "C:/OS Tuner/definitions";
    searchPaths << "C:/OS Tuner/docs for study/docsbyme/filesforstudy/config/ecuDef";
    searchPaths << "C:/OS Tuner/docs for study/docsbyme/speeduino";
    
    for (const QString& path : searchPaths) {
        QDir dir(path);
        if (!dir.exists()) continue;
        
        QStringList filters;
        filters << "*.ini";
        for (const QString& fileName : dir.entryList(filters, QDir::Files)) {
            QString fullPath = dir.absoluteFilePath(fileName);
            QFile file(fullPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                int lineCount = 0;
                while (!in.atEnd() && lineCount < 500) {
                    QString line = in.readLine().trimmed();
                    lineCount++;
                    
                    // Look for: signature = "..."
                    if (line.startsWith("signature", Qt::CaseInsensitive)) {
                        int eqIdx = line.indexOf('=');
                        if (eqIdx != -1) {
                            QString val = line.mid(eqIdx + 1).trimmed();
                            if (val.startsWith('"')) {
                                // Extract content between quotes
                                int firstQuote = val.indexOf('"');
                                int lastQuote = val.lastIndexOf('"');
                                if (lastQuote > firstQuote) {
                                    val = val.mid(firstQuote + 1, lastQuote - firstQuote - 1);
                                }
                            }
                            
                            // Check if signatures match (substring match)
                            if (!val.isEmpty() && (signature.contains(val, Qt::CaseInsensitive) || val.contains(signature, Qt::CaseInsensitive))) {
                                file.close();
                                return fullPath;
                            }
                        }
                    }
                }
                file.close();
            }
        }
    }
    return "";
}

void ProjectWizardOverlay::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
    QWidget::paintEvent(event);
}

void ProjectWizardOverlay::mousePressEvent(QMouseEvent *event) {
    // Prevent clicks from passing through
    event->accept();
}
