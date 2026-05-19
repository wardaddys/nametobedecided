#ifndef COLDSTARTWORKSPACE_H
#define COLDSTARTWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class ColdStartWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit ColdStartWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Cold Start Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "cold_start"; }
    QString workspaceLabel() const override { return "Cold Start"; }
    QStringList subsections() const override { return {"Cranking", "After-Start Enrichment (ASE)", "Warmup Enrichment (WUE)", "Cold Start Idle", "Cold Start Ignition", "Diagnostics"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Fueling → Cranking PW / ASE / WUE", "Idle Control → Cold Idle"};
    }
};

#endif // COLDSTARTWORKSPACE_H
