// E5: VVT2ControlWidget — adds the missing VVT2 control surface.
#include "VVT2ControlWidget.h"

#include "core/ECUSettingsManager.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

VVT2ControlWidget::VVT2ControlWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void VVT2ControlWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* hint = new QLabel(
        "VVT2 controls the exhaust cam on engines with dual VVT solenoids. "
        "Mode 'Off' disables the channel entirely.",
        this);
    hint->setStyleSheet("color: #888;");
    hint->setWordWrap(true);
    root->addWidget(hint);

    // --- General ---
    auto* generalBox = new QGroupBox("General", this);
    auto* generalForm = new QFormLayout(generalBox);

    m_mode = new QComboBox(generalBox);
    m_mode->addItems({"Off", "On/Off", "Open Loop", "Closed Loop"});
    generalForm->addRow("VVT2 Mode", m_mode);

    m_pwmFreq = new QSpinBox(generalBox);
    m_pwmFreq->setRange(1, 511);
    m_pwmFreq->setValue(30);
    m_pwmFreq->setSuffix(" Hz");
    generalForm->addRow("PWM Frequency", m_pwmFreq);

    m_hysteresis = new QSpinBox(generalBox);
    m_hysteresis->setRange(0, 20);
    m_hysteresis->setSuffix(" °");
    generalForm->addRow("Hysteresis", m_hysteresis);

    root->addWidget(generalBox);

    // --- Target table ---
    auto* tableBox = new QGroupBox("VVT2 Target Map (RPM × Load)", this);
    auto* tableLay = new QVBoxLayout(tableBox);
    m_targetTable = new QTableWidget(8, 8, tableBox);
    m_targetTable->setHorizontalHeaderLabels(
        {"500", "1500", "2500", "3500", "4500", "5500", "6500", "7500"});
    m_targetTable->setVerticalHeaderLabels(
        {"100", "85", "70", "55", "40", "30", "20", "10"});
    m_targetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableLay->addWidget(m_targetTable);
    root->addWidget(tableBox);

    connect(m_mode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int v) {
                if (m_settings && m_settings->hasSetting("vvt2Mode"))
                    m_settings->setValue("vvt2Mode", v, true);
            });
    connect(m_pwmFreq, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int v) {
                if (m_settings && m_settings->hasSetting("vvt2PWMFreq"))
                    m_settings->setValue("vvt2PWMFreq", v, true);
            });
    connect(m_hysteresis, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int v) {
                if (m_settings && m_settings->hasSetting("vvt2Hysteresis"))
                    m_settings->setValue("vvt2Hysteresis", v, true);
            });
    connect(m_targetTable, &QTableWidget::cellChanged,
            this, [this](int, int) { writeTargetTableToECU(); });
}

void VVT2ControlWidget::setSettingsManager(ECUSettingsManager* mgr) {
    m_settings = mgr;
    loadFromECU();
}

void VVT2ControlWidget::loadFromECU() {
    if (!m_settings) return;
    if (m_settings->hasSetting("vvt2Mode"))
        m_mode->setCurrentIndex(m_settings->getRawValue("vvt2Mode"));
    if (m_settings->hasSetting("vvt2PWMFreq"))
        m_pwmFreq->setValue(m_settings->getRawValue("vvt2PWMFreq"));
    if (m_settings->hasSetting("vvt2Hysteresis"))
        m_hysteresis->setValue(m_settings->getRawValue("vvt2Hysteresis"));

    // Populate target table cells from the vvt2Table table if loaded.
    const int rows = m_targetTable->rowCount();
    const int cols = m_targetTable->columnCount();
    m_targetTable->blockSignals(true);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QVariant v = m_settings->getTableValue("vvt2Table", r, c);
            const double val = v.isValid() ? v.toDouble() : 0.0;
            m_targetTable->setItem(r, c, new QTableWidgetItem(QString::number(val, 'f', 1)));
        }
    }
    m_targetTable->blockSignals(false);
}

void VVT2ControlWidget::writeTargetTableToECU() {
    if (!m_settings) return;
    const int rows = m_targetTable->rowCount();
    const int cols = m_targetTable->columnCount();
    QVector<QVector<double>> data(rows, QVector<double>(cols, 0.0));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto* it = m_targetTable->item(r, c);
            data[r][c] = it ? it->text().toDouble() : 0.0;
        }
    }
    m_settings->writeTable("vvt2Table", data);
}
