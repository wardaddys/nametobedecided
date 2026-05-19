#ifndef TRIGGERSWORKSPACE_H
#define TRIGGERSWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class TriggersWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit TriggersWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Triggers & Sync Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "triggers"; }
    QString workspaceLabel() const override { return "Triggers & Sync"; }
    QStringList subsections() const override { return {"Crank Trigger", "Cam Trigger", "Sync Behavior", "Diagnostics"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Engine Basics → Firing Order", "Sensors → Knock Sensor"};
    }
};

#endif // TRIGGERSWORKSPACE_H
