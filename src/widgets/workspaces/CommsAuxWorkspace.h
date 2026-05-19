#ifndef COMMSAUXWORKSPACE_H
#define COMMSAUXWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class CommsAuxWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit CommsAuxWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Communications & Auxiliary Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "comms"; }
    QString workspaceLabel() const override { return "Communications & Auxiliary"; }
    QStringList subsections() const override { return {"CAN Bus", "Secondary Serial", "Dashboard Output", "Datalogging Configuration", "OS Tuner Telemetry"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Inputs & Outputs → Pinout"};
    }
};

#endif // COMMSAUXWORKSPACE_H
