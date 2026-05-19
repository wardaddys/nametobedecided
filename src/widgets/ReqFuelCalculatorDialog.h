#ifndef REQFUELCALCULATORDIALOG_H
#define REQFUELCALCULATORDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

class ReqFuelCalculatorDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReqFuelCalculatorDialog(QWidget *parent = nullptr);

    double getBaseReqFuel() const;
    
    double getDisplacement() const;
    QString getDisplacementUnits() const;
    int getCylinders() const;
    double getInjectorFlow() const;
    QString getInjectorUnits() const;
    double getAFR() const;

    void setDisplacement(double val, const QString& units = "CC");
    void setCylinders(int val);
    void setInjectorFlow(double val, const QString& units = "cc/min");
    void setAFR(double val);

private slots:
    void calculate();

private:
    QDoubleSpinBox *m_displacementSpin;
    QComboBox *m_displacementUnits;
    
    QSpinBox *m_cylindersSpin;
    
    QDoubleSpinBox *m_injectorFlowSpin;
    QComboBox *m_injectorUnits;
    
    QDoubleSpinBox *m_afrSpin;
    
    QLabel *m_resultLabel;
    
    double m_baseReqFuel = 0.0;
};

#endif // REQFUELCALCULATORDIALOG_H
