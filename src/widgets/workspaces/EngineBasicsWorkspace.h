#ifndef ENGINEBASICSWORKSPACE_H
#define ENGINEBASICSWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class EngineBasicsWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit EngineBasicsWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Engine Basics Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "engine_basics"; }
    QString workspaceLabel() const override { return "Engine Basics"; }
    QStringList subsections() const override { return {}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Triggers & Sync", "Fueling → Injectors", "Ignition → Coil Configuration"};
    }
};

#endif // ENGINEBASICSWORKSPACE_H
