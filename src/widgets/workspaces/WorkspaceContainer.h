#ifndef WORKSPACECONTAINER_H
#define WORKSPACECONTAINER_H

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QMap>

class WorkspaceBase;

class WorkspaceContainer : public QWidget {
    Q_OBJECT

public:
    explicit WorkspaceContainer(QWidget* parent = nullptr);
    ~WorkspaceContainer() = default;

    void registerWorkspace(WorkspaceBase* workspace);
    void navigateTo(const QString& workspaceId, const QString& subsectionId = QString());
    void updateConditionalVisibility();

private slots:
    void onWorkspaceSelected(int row);

private:
    void setupUi();

    QListWidget* m_navRail;
    QStackedWidget* m_workspaceStack;
    QWidget* m_contextRail; // Placeholder for right side context (Phase 3)
    
    QMap<QString, WorkspaceBase*> m_workspaces;
    QMap<int, QString> m_rowToWorkspaceId;
};

#endif // WORKSPACECONTAINER_H
