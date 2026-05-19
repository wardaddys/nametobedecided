#include "ReqFuelCalculatorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QGroupBox>

ReqFuelCalculatorDialog::ReqFuelCalculatorDialog(QWidget *parent) 
    : QDialog(parent)
{
    setWindowTitle(tr("Required Fuel Calculator"));
    setModal(true);
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGroupBox *group = new QGroupBox(tr("Engine & Injector Specifications"), this);
    QGridLayout *grid = new QGridLayout(group);
    
    // Displacement
    grid->addWidget(new QLabel(tr("Engine Displacement:")), 0, 0);
    m_displacementSpin = new QDoubleSpinBox(this);
    m_displacementSpin->setRange(1.0, 20000.0);
    m_displacementSpin->setDecimals(1);
    m_displacementSpin->setValue(350.0);
    grid->addWidget(m_displacementSpin, 0, 1);
    
    m_displacementUnits = new QComboBox(this);
    m_displacementUnits->addItems({"CID", "CC"});
    m_displacementUnits->setCurrentText("CID");
    grid->addWidget(m_displacementUnits, 0, 2);
    
    // Cylinders
    grid->addWidget(new QLabel(tr("Number of Cylinders:")), 1, 0);
    m_cylindersSpin = new QSpinBox(this);
    m_cylindersSpin->setRange(1, 16);
    m_cylindersSpin->setValue(8);
    grid->addWidget(m_cylindersSpin, 1, 1);
    
    // Injector Flow
    grid->addWidget(new QLabel(tr("Injector Flow:")), 2, 0);
    m_injectorFlowSpin = new QDoubleSpinBox(this);
    m_injectorFlowSpin->setRange(1.0, 5000.0);
    m_injectorFlowSpin->setDecimals(1);
    m_injectorFlowSpin->setValue(30.0);
    grid->addWidget(m_injectorFlowSpin, 2, 1);
    
    m_injectorUnits = new QComboBox(this);
    m_injectorUnits->addItems({"lb/hr", "cc/min"});
    m_injectorUnits->setCurrentText("lb/hr");
    grid->addWidget(m_injectorUnits, 2, 2);
    
    // AFR
    grid->addWidget(new QLabel(tr("Air-Fuel Ratio:")), 3, 0);
    m_afrSpin = new QDoubleSpinBox(this);
    m_afrSpin->setRange(1.0, 25.0);
    m_afrSpin->setDecimals(2);
    m_afrSpin->setValue(14.7);
    grid->addWidget(m_afrSpin, 3, 1);
    
    mainLayout->addWidget(group);
    
    // Result
    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->addWidget(new QLabel(tr("Base Required Fuel (ms):")));
    m_resultLabel = new QLabel("0.0", this);
    QFont f = m_resultLabel->font();
    f.setBold(true);
    f.setPointSize(12);
    m_resultLabel->setFont(f);
    resLayout->addWidget(m_resultLabel);
    resLayout->addStretch();
    mainLayout->addLayout(resLayout);
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(btnBox);
    
    connect(btnBox, &QDialogButtonBox::accepted, this, [this]() {
        calculate();
        accept();
    });
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    auto updateCalc = [this]() { calculate(); };
    connect(m_displacementSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateCalc);
    connect(m_displacementUnits, &QComboBox::currentTextChanged, this, updateCalc);
    connect(m_cylindersSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, updateCalc);
    connect(m_injectorFlowSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateCalc);
    connect(m_injectorUnits, &QComboBox::currentTextChanged, this, updateCalc);
    connect(m_afrSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateCalc);
    
    calculate();
}

void ReqFuelCalculatorDialog::calculate() {
    double displacement = m_displacementSpin->value();
    if (m_displacementUnits->currentText() == "CC") {
        displacement /= 16.38706;
    }
    
    double injectorFlow = m_injectorFlowSpin->value();
    if (m_injectorUnits->currentText() == "cc/min") {
        injectorFlow /= 10.5;
    }
    
    double afr = m_afrSpin->value();
    int cylinders = m_cylindersSpin->value();
    
    if (displacement <= 0 || injectorFlow <= 0 || afr <= 0 || cylinders <= 0) {
        m_baseReqFuel = 0.0;
    } else {
        // TunerStudio magic constants extracted from an.java
        double d7 = 3.6e7;
        double d8 = 4.27793e-5;
        double d9 = d7 * displacement * d8;
        double d10 = cylinders * afr * injectorFlow;
        m_baseReqFuel = (d9 / d10) / 10.0;
    }
    
    m_resultLabel->setText(QString::number(m_baseReqFuel, 'f', 2));
}

double ReqFuelCalculatorDialog::getBaseReqFuel() const { return m_baseReqFuel; }
double ReqFuelCalculatorDialog::getDisplacement() const { return m_displacementSpin->value(); }
QString ReqFuelCalculatorDialog::getDisplacementUnits() const { return m_displacementUnits->currentText(); }
int ReqFuelCalculatorDialog::getCylinders() const { return m_cylindersSpin->value(); }
double ReqFuelCalculatorDialog::getInjectorFlow() const { return m_injectorFlowSpin->value(); }
QString ReqFuelCalculatorDialog::getInjectorUnits() const { return m_injectorUnits->currentText(); }
double ReqFuelCalculatorDialog::getAFR() const { return m_afrSpin->value(); }

void ReqFuelCalculatorDialog::setDisplacement(double val, const QString& units) {
    m_displacementSpin->setValue(val);
    m_displacementUnits->setCurrentText(units);
}
void ReqFuelCalculatorDialog::setCylinders(int val) {
    m_cylindersSpin->setValue(val);
}
void ReqFuelCalculatorDialog::setInjectorFlow(double val, const QString& units) {
    m_injectorFlowSpin->setValue(val);
    m_injectorUnits->setCurrentText(units);
}
void ReqFuelCalculatorDialog::setAFR(double val) {
    m_afrSpin->setValue(val);
}
