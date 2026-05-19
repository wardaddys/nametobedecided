#ifndef ENGINEPROTECTIONWORKSPACE_H
#define ENGINEPROTECTIONWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class EngineProtectionWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit EngineProtectionWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Engine Protection Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "engine_protection"; }
    QString workspaceLabel() const override { return "Engine Protection"; }
    QStringList subsections() const override { return {"Rev Limiters", "Overheat Protection", "Lean Protection", "Boost Cut", "Oil Pressure Protection", "Other"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Fueling → AFR Target", "Sensors → Battery Voltage"};
    }
};

#endif // ENGINEPROTECTIONWORKSPACE_H
