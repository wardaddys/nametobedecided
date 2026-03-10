#include "StartupDialog.h"
#include "utils/Settings.h"
#include "core/TunerProColors.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QGraphicsOpacityEffect>

StartupDialog::StartupDialog(QWidget *parent)
    : QDialog(parent), m_selectedAction(Exit) {
  setupUi();
  setWindowTitle("Welcome to TunerPro");
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  resize(520, 420); // Specified fixed size
}

StartupDialog::~StartupDialog() {}

void StartupDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(40, 40, 40, 40);
  mainLayout->setSpacing(0);
  
  // Custom Logo Mark (Drawing it later in paintEvent or via QLabel)
  m_logoLabel = new QLabel(this);
  m_logoLabel->setFixedSize(48, 48);
  // We'll draw the custom logo in a pixmap
  QPixmap logoPix(48, 48);
  logoPix.fill(Qt::transparent);
  QPainter p(&logoPix);
  p.setRenderHint(QPainter::Antialiasing);
  
  // 4 square waves, line width 2, radius 1
  p.setPen(QPen(QColor(TunerProColors::ACCENT), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  QPainterPath path;
  path.moveTo(4, 34);
  path.lineTo(14, 34);
  path.lineTo(14, 14);
  path.lineTo(34, 14);
  path.lineTo(34, 34);
  path.lineTo(44, 34);
  p.drawPath(path);
  m_logoLabel->setPixmap(logoPix);

  // Title 
  m_titleLabel = new QLabel("TUNERPRO / OPEN-SOURCE ECU", this);
  m_titleLabel->setStyleSheet(QString(
    "font-family: 'JetBrains Mono'; font-weight: bold; font-size: 24px; color: %1;"
  ).arg(TunerProColors::TEXT_PRIMARY));

  m_subtitleLabel = new QLabel("Select a project or create a new one to begin.", this);
  m_subtitleLabel->setStyleSheet(QString(
    "font-family: 'Inter'; font-size: 13px; color: %1;"
  ).arg(TunerProColors::TEXT_SECONDARY));

  mainLayout->addWidget(m_logoLabel);
  mainLayout->addSpacing(16);
  mainLayout->addWidget(m_titleLabel);
  mainLayout->addWidget(m_subtitleLabel);
  mainLayout->addSpacing(32);

  // Buttons Layout
  QVBoxLayout *btnLayout = new QVBoxLayout();
  btnLayout->setSpacing(8);
  
  QString btnStyle = QString(R"(
        QPushButton {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 6px;
            padding-left: 16px;
            color: %3;
            text-align: left;
            font-family: 'Inter';
            font-size: 13px;
            font-weight: 500;
            min-height: 44px;
            max-height: 44px;
        }
        QPushButton:hover {
            background-color: %4;
            border: 1px solid %5;
            color: %6;
        }
        QPushButton:pressed {
            background-color: %1;
        }
    )").arg(TunerProColors::BG_ELEVATED)
       .arg(TunerProColors::BORDER_DEFAULT)
       .arg(TunerProColors::TEXT_PRIMARY)
       .arg(TunerProColors::BG_INTERACTIVE)
       .arg(TunerProColors::ACCENT_BORDER)
       .arg(TunerProColors::ACCENT_BRIGHT);

  m_btnLast = new QPushButton("Open Last Project", this);
  m_btnLast->setStyleSheet(btnStyle);
  QString lastPath = Settings::getLastEcuDefPath();
  if (lastPath.isEmpty()) {
    m_btnLast->setEnabled(false);
    m_btnLast->setStyleSheet(m_btnLast->styleSheet() + "QPushButton { color: " + TunerProColors::TEXT_MUTED + "; }");
  }
  connect(m_btnLast, &QPushButton::clicked, this, &StartupDialog::onOpenLastClicked);

  m_btnOpen = new QPushButton("Browse For Project...", this);
  m_btnOpen->setStyleSheet(btnStyle);
  connect(m_btnOpen, &QPushButton::clicked, this, &StartupDialog::onOpenProjectClicked);

  m_btnExit = new QPushButton("Exit", this);
  m_btnExit->setStyleSheet(btnStyle);
  connect(m_btnExit, &QPushButton::clicked, this, &StartupDialog::onExitClicked);

  btnLayout->addWidget(m_btnLast);
  btnLayout->addWidget(m_btnOpen);
  btnLayout->addWidget(m_btnExit);
  
  mainLayout->addLayout(btnLayout);
  mainLayout->addStretch();
}

void StartupDialog::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  
  // Fill base color
  painter.fillRect(rect(), QColor(TunerProColors::BG_BASE));

  // Draw radial gradient mask for the "PCB Trace" decoration approximation
  QRadialGradient gradient(rect().bottomRight(), rect().width() * 0.8);
  gradient.setColorAt(0, QColor(TunerProColors::ACCENT_GLOW)); // Slight glow 
  gradient.setColorAt(1, Qt::transparent);
  
  painter.fillRect(rect(), gradient);
  
  // Outer Border
  painter.setPen(QPen(QColor(TunerProColors::BORDER_DEFAULT), 1));
  painter.drawRect(rect().adjusted(0,0,-1,-1));
}

void StartupDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  setupAnimations();
}

void StartupDialog::setupAnimations() {
  // Set initial states for animation execution
  m_logoLabel->setGraphicsEffect(new QGraphicsOpacityEffect(this));
  m_titleLabel->setGraphicsEffect(new QGraphicsOpacityEffect(this));
  m_subtitleLabel->setGraphicsEffect(new QGraphicsOpacityEffect(this));
  m_btnLast->setGraphicsEffect(new QGraphicsOpacityEffect(this));
  m_btnOpen->setGraphicsEffect(new QGraphicsOpacityEffect(this));
  m_btnExit->setGraphicsEffect(new QGraphicsOpacityEffect(this));

  auto *logoEffect = static_cast<QGraphicsOpacityEffect*>(m_logoLabel->graphicsEffect());
  auto *titleEffect = static_cast<QGraphicsOpacityEffect*>(m_titleLabel->graphicsEffect());
  auto *subEffect = static_cast<QGraphicsOpacityEffect*>(m_subtitleLabel->graphicsEffect());
  auto *lastEffect = static_cast<QGraphicsOpacityEffect*>(m_btnLast->graphicsEffect());
  auto *openEffect = static_cast<QGraphicsOpacityEffect*>(m_btnOpen->graphicsEffect());
  auto *exitEffect = static_cast<QGraphicsOpacityEffect*>(m_btnExit->graphicsEffect());

  logoEffect->setOpacity(0.0);
  titleEffect->setOpacity(0.0);
  subEffect->setOpacity(0.0);
  lastEffect->setOpacity(0.0);
  openEffect->setOpacity(0.0);
  exitEffect->setOpacity(0.0);

  QSequentialAnimationGroup *seq = new QSequentialAnimationGroup(this);

  // 1. Logo
  QPropertyAnimation *animLogo = new QPropertyAnimation(logoEffect, "opacity");
  animLogo->setDuration(400);
  animLogo->setStartValue(0.0);
  animLogo->setEndValue(1.0);
  seq->addAnimation(animLogo);

  // 2. Titles (delay via pause isn't easily possible here unless using pause(time) so we do it manual or directly add)
  QParallelAnimationGroup *grpTitles = new QParallelAnimationGroup();
  
  QPropertyAnimation *animTitleOp = new QPropertyAnimation(titleEffect, "opacity");
  animTitleOp->setDuration(300);
  animTitleOp->setStartValue(0.0);
  animTitleOp->setEndValue(1.0);

  QPropertyAnimation *animTitlePos = new QPropertyAnimation(m_titleLabel, "pos");
  animTitlePos->setDuration(300);
  animTitlePos->setStartValue(m_titleLabel->pos() + QPoint(0, 10)); // slide up 10px
  animTitlePos->setEndValue(m_titleLabel->pos());
  animTitlePos->setEasingCurve(QEasingCurve::OutCubic);

  QPropertyAnimation *animSubOp = new QPropertyAnimation(subEffect, "opacity");
  animSubOp->setDuration(300);
  animSubOp->setStartValue(0.0);
  animSubOp->setEndValue(1.0);

  grpTitles->addAnimation(animTitleOp);
  grpTitles->addAnimation(animTitlePos);
  grpTitles->addAnimation(animSubOp);
  seq->addAnimation(grpTitles);

  // 3. Staggered buttons
  QPropertyAnimation *aBtn1 = new QPropertyAnimation(lastEffect, "opacity");
  aBtn1->setDuration(300);
  aBtn1->setStartValue(0.0);
  aBtn1->setEndValue(1.0);
  
  QPropertyAnimation *aBtn2 = new QPropertyAnimation(openEffect, "opacity");
  aBtn2->setDuration(300);
  aBtn2->setStartValue(0.0);
  aBtn2->setEndValue(1.0);
  
  QPropertyAnimation *aBtn3 = new QPropertyAnimation(exitEffect, "opacity");
  aBtn3->setDuration(300);
  aBtn3->setStartValue(0.0);
  aBtn3->setEndValue(1.0);
  
  seq->addAnimation(aBtn1);
  seq->addAnimation(aBtn2);
  seq->addAnimation(aBtn3);

  seq->start(QAbstractAnimation::DeleteWhenStopped);
}

void StartupDialog::onCreateNewClicked() {
  m_selectedAction = CreateNew;
  accept();
}

void StartupDialog::onOpenProjectClicked() {
  m_selectedAction = OpenProject;
  accept();
}

void StartupDialog::onOpenLastClicked() {
  m_selectedAction = OpenLast;
  accept();
}

void StartupDialog::onExitClicked() {
  m_selectedAction = Exit;
  reject();
}
