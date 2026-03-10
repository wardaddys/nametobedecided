#include "CommandButton.h"
#include <QMessageBox>
#include <QDebug>

CommandButton::CommandButton(const QString& label, QWidget* parent)
    : QPushButton(label, parent)
    , m_clickFlags(NoFlag)
{
    connect(this, &QPushButton::clicked, this, &CommandButton::onClicked);
    
    // Style the button
    setStyleSheet(
        "QPushButton {"
        "  background-color: #2d3748;"
        "  color: #e2e8f0;"
        "  border: 1px solid #4a5568;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #4a5568;"
        "  border-color: #00bcd4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1a202c;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #1a202c;"
        "  color: #718096;"
        "  border-color: #2d3748;"
        "}"
    );
}

CommandButton::~CommandButton() = default;

void CommandButton::setCommand(const QByteArray& command) {
    m_commands.clear();
    m_commands.append(command);
}

void CommandButton::setCommandChain(const QList<QByteArray>& commands) {
    m_commands = commands;
}

void CommandButton::setEnabledCondition(const QString& expression) {
    m_enabledCondition = expression;
}

void CommandButton::setClickFlags(ClickFlags flags) {
    m_clickFlags = flags;
}

void CommandButton::setClickMessage(const QString& message) {
    m_clickMessage = message;
}

void CommandButton::setCommandSender(CommandSender sender) {
    m_commandSender = sender;
}

void CommandButton::setConditionEvaluator(ConditionEvaluator evaluator) {
    m_conditionEvaluator = evaluator;
    updateEnabledState();
}

void CommandButton::executeCommand() {
    if (m_commands.isEmpty()) {
        qWarning() << "CommandButton: No command set";
        emit commandExecuted(false);
        return;
    }

    if (!m_commandSender) {
        qWarning() << "CommandButton: No command sender set";
        emit commandExecuted(false);
        return;
    }

    bool success = true;
    for (const QByteArray& cmd : m_commands) {
        if (!m_commandSender(cmd)) {
            success = false;
            break;
        }
    }

    emit commandExecuted(success);

    // Show message if flag set
    if (m_clickFlags & ShowMessageOnClick && !m_clickMessage.isEmpty()) {
        QMessageBox::information(this, "Command", m_clickMessage);
    }

    // Close dialog if flag set
    if (m_clickFlags & CloseDialogOnClick) {
        emit requestDialogClose();
    }
}

void CommandButton::updateEnabledState() {
    if (m_enabledCondition.isEmpty()) {
        setEnabled(true);
        return;
    }

    if (m_conditionEvaluator) {
        bool enabled = m_conditionEvaluator(m_enabledCondition);
        setEnabled(enabled);
    }
}

bool CommandButton::shouldExecuteOnClose() const {
    if (m_clickFlags & ClickOnClose) {
        return true;
    }
    if (m_clickFlags & ClickOnCloseIfEnabled && isEnabled()) {
        return true;
    }
    if (m_clickFlags & ClickOnCloseIfDisabled && !isEnabled()) {
        return true;
    }
    return false;
}

void CommandButton::onClicked() {
    executeCommand();
}
