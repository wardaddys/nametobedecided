#ifndef SENSORSWORKSPACE_H
#define SENSORSWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class SensorsWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit SensorsWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Sensors & Calibration Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "sensors"; }
    QString workspaceLabel() const override { return "Sensors & Calibration"; }
    
    QStringList subsections() const override {
        return {
            "MAP Sensor", "TPS", "CLT (Coolant Temp)", "IAT (Intake Air Temp)",
            "O2 / Lambda", "Knock Sensor", "Battery Voltage", "Optional Sensors"
        };
    }
    
    QWidget* createSubsectionWidget(const QString& /*subsectionId*/) override {
        return new QWidget(); // Placeholder
    }
    
    QStringList crossReferences() const override {
        return {"Triggers & Sync", "Fueling", "Ignition"};
    }
};

#endif // SENSORSWORKSPACE_H
