#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>

class StartupDialog : public QDialog {
  Q_OBJECT

public:
  explicit StartupDialog(QWidget *parent = nullptr);
  ~StartupDialog();

  enum Action { CreateNew, OpenProject, OpenLast, Exit };

  Action getSelectedAction() const { return m_selectedAction; }

protected:
  void showEvent(QShowEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private slots:
  void onCreateNewClicked();
  void onOpenProjectClicked();
  void onOpenLastClicked();
  void onExitClicked();

private:
  Action m_selectedAction;
  void setupUi();
  void setupAnimations();

  QLabel *m_logoLabel;
  QLabel *m_titleLabel;
  QLabel *m_subtitleLabel;
  QPushButton *m_btnLast;
  QPushButton *m_btnOpen;
  QPushButton *m_btnExit;
};

#endif // STARTUPDIALOG_H
