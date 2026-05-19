#ifndef PROJECTWIZARDOVERLAY_H
#define PROJECTWIZARDOVERLAY_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include "../utils/FileDownloader.h"

class SerialManager; // Forward declaration

class ProjectWizardOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ProjectWizardOverlay(QWidget *parent = nullptr);

    void setSerialManager(SerialManager* serialManager) { m_serialManager = serialManager; }
    void startWizard();

signals:
    void projectCreated(const QString& name, const QString& path);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void nextStep();
    void prevStep();
    void finishWizard();
    void detectFirmware();

private:
    void setupUi();
    void updateButtons();
    void buildDynamicConfigStep();
    QString findMatchingIni(const QString& signature);

    QWidget *m_panel;
    QStackedWidget *m_stack;

    // Step 1: Details
    QLineEdit *m_nameEdit;
    QLineEdit *m_dirEdit;
    QLineEdit *m_firmwareEdit;
    QTextEdit *m_descEdit;
    QPushButton *m_detectBtn;
    QPushButton *m_browseIniBtn;
    QString m_selectedIniPath;

    // Step 2: Config
    QWidget *m_step2Widget;
    QMap<QString, QComboBox*> m_dynamicCombos;
    FileDownloader *m_downloader;

    // Step 3: Communications
    QComboBox *m_comPortCombo;
    QComboBox *m_baudCombo;

    // Navigation
    QPushButton *m_cancelBtn;
    QPushButton *m_backBtn;
    QPushButton *m_nextBtn;
    QLabel *m_statusLabel;

    SerialManager* m_serialManager;
};

#endif // PROJECTWIZARDOVERLAY_H
