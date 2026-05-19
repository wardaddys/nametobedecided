#ifndef INPUTSOUTPUTSWORKSPACE_H
#define INPUTSOUTPUTSWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class InputsOutputsWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit InputsOutputsWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Inputs & Outputs Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "io"; }
    QString workspaceLabel() const override { return "Inputs & Outputs"; }
    QStringList subsections() const override { return {"Pinout", "Tachometer Output", "MIL / Check Engine Light", "Fuel Pump Control", "Fan Control", "Accessory Outputs"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Pinout page"};
    }
};

#endif // INPUTSOUTPUTSWORKSPACE_H
