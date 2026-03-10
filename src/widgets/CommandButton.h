#ifndef COMMANDBUTTON_H
#define COMMANDBUTTON_H

#include <QPushButton>
#include <QList>
#include <QString>
#include <functional>

/**
 * @brief Command Button widget for TunerPro
 * 
 * A button that sends controller commands when clicked.
 * Supports:
 * - Single commands
 * - Command chaining (cmd1, cmd2, cmd3)
 * - Enabled condition expressions
 * - Click flags (clickOnClose, clickOnCloseIfEnabled, clickOnCloseIfDisabled)
 * 
 * Example INI usage:
 * commandButton = "Start Test Mode", cmdEnterTestMode, { rpm > 0 }
 * commandButton = "Stop Test Mode", cmdStopTestmode, { rpm == 0 }, clickOnCloseIfEnabled
 */
class CommandButton : public QPushButton {
    Q_OBJECT

public:
    enum ClickFlag {
        NoFlag = 0,
        ClickOnClose = 1,           // Always send on dialog close
        ClickOnCloseIfEnabled = 2,  // Send on close only if enabled
        ClickOnCloseIfDisabled = 4, // Send on close only if disabled
        CloseDialogOnClick = 8,     // Close parent dialog after click
        ShowMessageOnClick = 16     // Show message after click
    };
    Q_DECLARE_FLAGS(ClickFlags, ClickFlag)

    using CommandSender = std::function<bool(const QByteArray&)>;
    using ConditionEvaluator = std::function<bool(const QString&)>;

    explicit CommandButton(const QString& label, QWidget* parent = nullptr);
    ~CommandButton() override;

    // Set the command(s) to send
    void setCommand(const QByteArray& command);
    void setCommandChain(const QList<QByteArray>& commands);

    // Set enabled condition expression
    void setEnabledCondition(const QString& expression);

    // Set click flags
    void setClickFlags(ClickFlags flags);
    ClickFlags clickFlags() const { return m_clickFlags; }

    // Set message to show after click (if ShowMessageOnClick flag set)
    void setClickMessage(const QString& message);

    // Set command sender callback
    void setCommandSender(CommandSender sender);

    // Set condition evaluator callback
    void setConditionEvaluator(ConditionEvaluator evaluator);

    // Execute the command(s)
    void executeCommand();

    // Update enabled state based on condition
    void updateEnabledState();

    // Check if should execute on close
    bool shouldExecuteOnClose() const;

signals:
    void commandExecuted(bool success);
    void requestDialogClose();

private slots:
    void onClicked();

private:
    QList<QByteArray> m_commands;
    QString m_enabledCondition;
    ClickFlags m_clickFlags;
    QString m_clickMessage;
    CommandSender m_commandSender;
    ConditionEvaluator m_conditionEvaluator;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CommandButton::ClickFlags)

#endif // COMMANDBUTTON_H
