#ifndef BOOSTWORKSPACE_H
#define BOOSTWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class BoostWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit BoostWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Boost & Aspiration Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "boost"; }
    QString workspaceLabel() const override { return "Boost & Aspiration"; }
    QStringList subsections() const override { return {"Boost Targets", "Wastegate Control", "Anti-Lag", "Launch Control"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Sensors → MAP", "Engine Protection → Boost Cut", "Ignition → Knock Retard"};
    }
    
    // Will conditionally be false later if engine is NA
    bool isApplicable() const override { return true; } 
};

#endif // BOOSTWORKSPACE_H
