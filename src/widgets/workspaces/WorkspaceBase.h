#ifndef WORKSPACEBASE_H
#define WORKSPACEBASE_H

#include <QWidget>
#include <QStringList>

class WorkspaceBase : public QWidget {
    Q_OBJECT

public:
    explicit WorkspaceBase(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~WorkspaceBase() = default;

    virtual QString workspaceId() const = 0;
    virtual QString workspaceLabel() const = 0;
    
    // Sub-sections available in this workspace
    virtual QStringList subsections() const = 0;
    
    // Create the widget for a specific sub-section
    virtual QWidget* createSubsectionWidget(const QString& subsectionId) = 0;
    
    // Cross references shown at the bottom
    virtual QStringList crossReferences() const = 0;
    
    // Whether this workspace should be visible based on engine config
    virtual bool isApplicable() const { return true; }

signals:
    void navigateToWorkspace(const QString& workspaceId, const QString& subsectionId);
};

#endif // WORKSPACEBASE_H
