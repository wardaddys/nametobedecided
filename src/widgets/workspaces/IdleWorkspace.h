#ifndef IDLEWORKSPACE_H
#define IDLEWORKSPACE_H

#include "WorkspaceBase.h"
#include <QVBoxLayout>
#include <QLabel>

class IdleWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit IdleWorkspace(QWidget* parent = nullptr) : WorkspaceBase(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Idle Control Workspace (Content coming in Phase 2)", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #ccc; font-size: 18px;");
        layout->addWidget(label);
    }

    QString workspaceId() const override { return "idle"; }
    QString workspaceLabel() const override { return "Idle Control"; }
    QStringList subsections() const override { return {"Idle Targets", "Idle Air Control", "Compensations", "Idle Ignition"}; }
    QWidget* createSubsectionWidget(const QString&) override { return new QWidget(); }
    QStringList crossReferences() const override {
        return {"Cold Start → Cold Idle", "Inputs & Outputs → Fan Control"};
    }
};

#endif // IDLEWORKSPACE_H
