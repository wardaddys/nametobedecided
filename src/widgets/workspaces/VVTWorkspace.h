#ifndef VVTWORKSPACE_H
#define VVTWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class VVTWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit VVTWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("VVT Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "vvt"; }
    QString workspaceLabel() const override { return "VVT"; }
    QStringList subsections() const override { return {"VVT Hardware", "VVT Targets", "VVT Control", "Diagnostics"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Triggers & Sync", "Fueling → VE Table"};
    }
    
    // Will conditionally be false later if VVT not configured
    bool isApplicable() const override { return true; } 
};

#endif // VVTWORKSPACE_H
