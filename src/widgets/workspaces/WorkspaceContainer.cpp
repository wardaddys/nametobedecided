#include "WorkspaceContainer.h"
#include "WorkspaceBase.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QDebug>

WorkspaceContainer::WorkspaceContainer(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void WorkspaceContainer::setupUi() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left Navigation Rail
    m_navRail = new QListWidget(this);
    m_navRail->setFixedWidth(200);
    m_navRail->setStyleSheet(
        "QListWidget { background: #2b2b2b; border: none; border-right: 1px solid #444; }"
        "QListWidget::item { padding: 12px 16px; color: #ccc; border-bottom: 1px solid #333; }"
        "QListWidget::item:selected { background: #3d5afe; color: #fff; font-weight: bold; }"
        "QListWidget::item:hover { background: #3a3a3a; }"
    );
    connect(m_navRail, &QListWidget::currentRowChanged, this, &WorkspaceContainer::onWorkspaceSelected);

    // Center Workspace Stack
    m_workspaceStack = new QStackedWidget(this);
    m_workspaceStack->setStyleSheet("QStackedWidget { background: #1e1e1e; }");

    // Right Context Rail (Placeholder for now)
    m_contextRail = new QWidget(this);
    m_contextRail->setFixedWidth(250);
    m_contextRail->setStyleSheet("QWidget { background: #252525; border-left: 1px solid #444; }");
    QVBoxLayout* contextLayout = new QVBoxLayout(m_contextRail);
    QLabel* contextTitle = new QLabel("Context & Related", m_contextRail);
    contextTitle->setStyleSheet("color: #888; font-size: 14px; font-weight: bold;");
    contextLayout->addWidget(contextTitle);
    contextLayout->addStretch();

    mainLayout->addWidget(m_navRail);
    mainLayout->addWidget(m_workspaceStack, 1);
    mainLayout->addWidget(m_contextRail);
}

void WorkspaceContainer::registerWorkspace(WorkspaceBase* workspace) {
    if (!workspace) return;
    
    QString id = workspace->workspaceId();
    m_workspaces.insert(id, workspace);
    m_workspaceStack->addWidget(workspace);
    
    connect(workspace, &WorkspaceBase::navigateToWorkspace, this, [this](const QString& wId, const QString& subId) {
        navigateTo(wId, subId);
    });
    
    updateConditionalVisibility();
}

void WorkspaceContainer::updateConditionalVisibility() {
    // Rebuild the nav list based on isApplicable()
    QString currentWorkspaceId;
    if (m_navRail->currentRow() >= 0 && m_rowToWorkspaceId.contains(m_navRail->currentRow())) {
        currentWorkspaceId = m_rowToWorkspaceId[m_navRail->currentRow()];
    }
    
    m_navRail->clear();
    m_rowToWorkspaceId.clear();
    
    // Order based on the locked taxonomy list
    QStringList orderedWorkspaces = {
        "engine_basics", "sensors", "triggers", "fueling", "ignition", 
        "cold_start", "idle", "boost", "vvt", "engine_protection", "io", "comms"
    };
    
    int row = 0;
    int targetRow = 0;
    
    for (const QString& id : orderedWorkspaces) {
        if (m_workspaces.contains(id)) {
            WorkspaceBase* ws = m_workspaces[id];
            if (ws->isApplicable()) {
                QListWidgetItem* item = new QListWidgetItem(ws->workspaceLabel(), m_navRail);
                m_rowToWorkspaceId[row] = id;
                if (id == currentWorkspaceId) {
                    targetRow = row;
                }
                row++;
            }
        }
    }
    
    if (row > 0) {
        m_navRail->setCurrentRow(targetRow);
    }
}

void WorkspaceContainer::onWorkspaceSelected(int row) {
    if (row < 0 || !m_rowToWorkspaceId.contains(row)) return;
    
    QString workspaceId = m_rowToWorkspaceId[row];
    if (m_workspaces.contains(workspaceId)) {
        m_workspaceStack->setCurrentWidget(m_workspaces[workspaceId]);
    }
}

void WorkspaceContainer::navigateTo(const QString& workspaceId, const QString& subsectionId) {
    for (int i = 0; i < m_navRail->count(); ++i) {
        if (m_rowToWorkspaceId.contains(i) && m_rowToWorkspaceId[i] == workspaceId) {
            m_navRail->setCurrentRow(i);
            break;
        }
    }
    
    // Pass subsection logic down to the workspace (will implement in Phase 2)
    // if (!subsectionId.isEmpty() && m_workspaces.contains(workspaceId)) {
    //     m_workspaces[workspaceId]->setSubsection(subsectionId);
    // }
}
