// E1: DecoderConfigWidget — full 28-pattern trigger/decoder setup.
//
// Pattern names match decoders.h in the Speeduino firmware (2025.01).
// Settings constant names follow the canonical Speeduino INI: triggerPattern,
// numTeeth, missingTeeth, trigFilter, trigEdge, secondaryTrigEdge,
// thirdTrigEdge, sparkDur (not a trigger field but kept for compactness in
// older trees), and skipRevolutions.
#include "DecoderConfigWidget.h"

#include "core/ECUSettingsManager.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

DecoderConfigWidget::DecoderConfigWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    wireSignals();
}

void DecoderConfigWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* patternGroup = new QGroupBox("Trigger Pattern", this);
    auto* patternForm  = new QFormLayout(patternGroup);

    m_trigPattern = new QComboBox(patternGroup);
    // Order matters — Speeduino's DECODER_* enum values are the list index.
    const QStringList patterns = {
        "Missing Tooth (0)",         "Basic Distributor (1)",   "Dual Wheel (2)",
        "GM 7X (3)",                 "Mitsubishi 4G63 (4)",     "GM 24X (5)",
        "Jeep 2000 (6)",             "Audi 135 (7)",            "Honda D17 (8)",
        "Mazda Miata 99-05 (9)",     "Mazda AU (10)",           "Non-360 (11)",
        "Nissan 360 (12)",           "Subaru 6+7 (13)",         "Daihatsu +1 (14)",
        "Harley-Davidson (15)",      "36-2-2-2 (16)",           "36-2-1 (17)",
        "420A (18)",                 "Weber-Marelli (19)",      "Ford ST170 (20)",
        "Suzuki DRZ400 (21)",        "Chrysler NGC (22)",       "Yamaha VMAX (23)",
        "Renault Renix (24)",        "Rover MEMS (25)",         "Suzuki K6A (26)",
        "Honda J32 (27)"
    };
    m_trigPattern->addItems(patterns);
    patternForm->addRow("Decoder", m_trigPattern);

    m_wheelTeeth = new QSpinBox(patternGroup);
    m_wheelTeeth->setRange(1, 255);
    m_wheelTeeth->setValue(36);
    patternForm->addRow("Wheel Teeth (total)", m_wheelTeeth);

    m_missingTooth = new QSpinBox(patternGroup);
    m_missingTooth->setRange(0, 8);
    m_missingTooth->setValue(1);
    patternForm->addRow("Missing Teeth", m_missingTooth);

    m_skipRevolutions = new QSpinBox(patternGroup);
    m_skipRevolutions->setRange(0, 5);
    m_skipRevolutions->setValue(0);
    patternForm->addRow("Skip Revolutions", m_skipRevolutions);

    root->addWidget(patternGroup);

    auto* edgeGroup = new QGroupBox("Trigger Edges & Filter", this);
    auto* edgeForm  = new QFormLayout(edgeGroup);

    m_trigEdge = new QComboBox(edgeGroup);
    m_trigEdge->addItems({"Rising", "Falling"});
    edgeForm->addRow("Primary Edge", m_trigEdge);

    m_secTrigEdge = new QComboBox(edgeGroup);
    m_secTrigEdge->addItems({"Rising", "Falling"});
    edgeForm->addRow("Secondary Edge (cam)", m_secTrigEdge);

    m_thirdTrigEdge = new QComboBox(edgeGroup);
    m_thirdTrigEdge->addItems({"Rising", "Falling"});
    edgeForm->addRow("Third Edge (cam2)", m_thirdTrigEdge);

    m_trigFilter = new QComboBox(edgeGroup);
    m_trigFilter->addItems({"Off", "Lite", "Medium", "Aggressive"});
    edgeForm->addRow("Trigger Filter Level", m_trigFilter);

    root->addWidget(edgeGroup);
    root->addStretch();

    auto* hint = new QLabel(
        "Trigger settings drive sync detection. Wrong values prevent the "
        "engine from cranking. Verify with a Tooth Logger capture before "
        "first-start.",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888;");
    root->addWidget(hint);
}

void DecoderConfigWidget::wireSignals() {
    auto write = [this](const QString& name, const QVariant& v) {
        if (m_settings && m_settings->hasSetting(name)) {
            m_settings->setValue(name, v, true);
        }
    };
    connect(m_trigPattern, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [write](int v) { write("triggerPattern", v); });
    connect(m_wheelTeeth, QOverload<int>::of(&QSpinBox::valueChanged),
            [write](int v) { write("numTeeth", v); });
    connect(m_missingTooth, QOverload<int>::of(&QSpinBox::valueChanged),
            [write](int v) { write("missingTeeth", v); });
    connect(m_trigFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [write](int v) { write("trigFilter", v); });
    connect(m_trigEdge, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [write](int v) { write("trigEdge", v); });
    connect(m_secTrigEdge, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [write](int v) { write("secondaryTrigEdge", v); });
    connect(m_thirdTrigEdge, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [write](int v) { write("thirdTrigEdge", v); });
    connect(m_skipRevolutions, QOverload<int>::of(&QSpinBox::valueChanged),
            [write](int v) { write("skipRevolutions", v); });
}

void DecoderConfigWidget::setSettingsManager(ECUSettingsManager* mgr) {
    m_settings = mgr;
    loadFromECU();
}

void DecoderConfigWidget::loadFromECU() {
    if (!m_settings) return;
    auto load = [this](const QString& name, auto* widget) {
        if (!m_settings->hasSetting(name)) return;
        const int v = m_settings->getRawValue(name);
        if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            if (v >= 0 && v < combo->count()) combo->setCurrentIndex(v);
        } else if (auto* spin = qobject_cast<QSpinBox*>(widget)) {
            spin->setValue(v);
        }
    };
    load("triggerPattern",   m_trigPattern);
    load("numTeeth",         m_wheelTeeth);
    load("missingTeeth",     m_missingTooth);
    load("trigFilter",       m_trigFilter);
    load("trigEdge",         m_trigEdge);
    load("secondaryTrigEdge", m_secTrigEdge);
    load("thirdTrigEdge",    m_thirdTrigEdge);
    load("skipRevolutions",  m_skipRevolutions);
}
