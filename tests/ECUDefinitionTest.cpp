// =============================================================================
// G2: ECUDefinition INI parser tests
//
// Locks the working surface of the INI parser before Group C adds new section
// parsers (SettingGroups, PcVariables, Datalog, GaugeConfigurations,
// CurveEditor, Menu, UserDefined) so we can be sure none of them break the
// existing five-section behaviour.
//
// Uses an inline fixture string (hermetic, no path dependency) plus the real
// MS1 Extra INI shipped with the project as a smoke test.
// =============================================================================

#include <QtTest/QtTest>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

#include "core/ECUDefinition.h"

class ECUDefinitionTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void parsesTunerStudioSignature();
  void parsesScalarConstant();
  void parsesBitsConstant();
  void parsesArrayConstant();
  void parsesOutputChannelOffsets();
  void parsesControllerCommandBytes();
  void parsesTableEditorBlock();

  void conditionalIfSet_skipsFalseBranch();
  void conditionalIfSet_keepsTrueBranch();
  void conditionalNestedIfElse_correctBranch();

  void loadResetsState_noAccumulation();

  void smoke_realMs1ExtraIniFixture();

private:
  QString writeFixture(const QByteArray& content);
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
QString ECUDefinitionTest::writeFixture(const QByteArray& content) {
  auto* tmp = new QTemporaryFile(this);
  tmp->setAutoRemove(true);
  // The QTemporaryFile created via new QTemporaryFile is parented; it lives
  // until the test object is destroyed.
  if (tmp->open()) {
    tmp->write(content);
    tmp->flush();
  }
  const QString path = tmp->fileName();
  tmp->close();
  return path;
}

void ECUDefinitionTest::initTestCase() {
  // No global setup required — every test builds its own ECUDefinition.
}

// ---------------------------------------------------------------------------
// Section parsing
// ---------------------------------------------------------------------------
void ECUDefinitionTest::parsesTunerStudioSignature() {
  const QByteArray ini =
      "[TunerStudio]\n"
      "  signature = \"speeduino 202501\"\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));
  QCOMPARE(def.getSignature(), QString("speeduino 202501"));
}

void ECUDefinitionTest::parsesScalarConstant() {
  const QByteArray ini =
      "[Constants]\n"
      "page = 1\n"
      "mapAt5V = scalar, U16, 8, \"kPa\", 0.1, 0, 0, 600, 1\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& consts = def.getConstants();
  QVERIFY(consts.contains("mapAt5V"));
  const auto& c = consts.value("mapAt5V");
  QCOMPARE(c.paramClass, QString("scalar"));
  QCOMPARE(c.type, QString("U16"));
  QCOMPARE(c.page, 1);
  QCOMPARE(c.offset, 8);
  QCOMPARE(c.units, QString("kPa"));
  QCOMPARE(c.scale, 0.1);
  QCOMPARE(c.min, 0.0);
  QCOMPARE(c.max, 600.0);
}

void ECUDefinitionTest::parsesBitsConstant() {
  const QByteArray ini =
      "[Constants]\n"
      "page = 2\n"
      "engineType = bits, U08, 0, [0:0], \"Even fire\", \"Odd fire\"\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& consts = def.getConstants();
  QVERIFY(consts.contains("engineType"));
  const auto& c = consts.value("engineType");
  QCOMPARE(c.paramClass, QString("bits"));
  QCOMPARE(c.type, QString("U08"));
  QVERIFY(c.bitField.options.size() >= 2);
  QCOMPARE(c.bitField.options.value(0), QString("Even fire"));
  QCOMPARE(c.bitField.options.value(1), QString("Odd fire"));
}

void ECUDefinitionTest::parsesArrayConstant() {
  const QByteArray ini =
      "[Constants]\n"
      "page = 3\n"
      "veBins = array, U08, 16, [16], \"%\", 1, 0, 0, 255, 0\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& consts = def.getConstants();
  QVERIFY(consts.contains("veBins"));
  const auto& c = consts.value("veBins");
  QCOMPARE(c.paramClass, QString("array"));
  QCOMPARE(c.cols, 16);
}

void ECUDefinitionTest::parsesOutputChannelOffsets() {
  const QByteArray ini =
      "[OutputChannels]\n"
      "ochBlockSize = 130\n"
      "rpm   = scalar, U16, 14, \"RPM\", 1, 0\n"
      "clt   = scalar, S16, 6,  \"C\",   0.1, -270\n"
      "tps   = scalar, U08, 24, \"%\",   0.5, 0\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& channels = def.getOutputChannels();
  QVERIFY(channels.contains("rpm"));
  QVERIFY(channels.contains("clt"));
  QVERIFY(channels.contains("tps"));
  QCOMPARE(channels.value("rpm").offset, 14);
  QCOMPARE(channels.value("tps").offset, 24);
}

void ECUDefinitionTest::parsesControllerCommandBytes() {
  const QByteArray ini =
      "[ControllerCommands]\n"
      "cmdBurn = 0x62\n"
      "cmdReset = \"U\"\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& cmds = def.getControllerCommands();
  QVERIFY(cmds.contains("cmdBurn"));
  QVERIFY(cmds.contains("cmdReset"));
}

void ECUDefinitionTest::parsesTableEditorBlock() {
  const QByteArray ini =
      "[Constants]\n"
      "page = 4\n"
      "veTable = array, U08, 0, [16x16], \"%\", 1, 0, 0, 255, 0\n"
      "rpmBins = array, U08, 256, [16], \"RPM\", 100, 0, 0, 25500, 0\n"
      "mapBins = array, U08, 272, [16], \"kPa\", 1, 0, 0, 255, 0\n"
      "[TableEditor]\n"
      "table = veMap, veTable, \"VE Table\"\n"
      "  xBins = rpmBins, rpm\n"
      "  yBins = mapBins, map\n"
      "  zBins = veTable\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));

  const auto& tables = def.getTableDefinitions();
  QVERIFY(tables.contains("veMap") || tables.contains("veTable"));
}

// ---------------------------------------------------------------------------
// Conditional parsing
// ---------------------------------------------------------------------------
void ECUDefinitionTest::conditionalIfSet_skipsFalseBranch() {
  const QByteArray ini =
      "#unset FEATURE_X\n"
      "[Constants]\n"
      "page = 1\n"
      "#if FEATURE_X\n"
      "  shouldNotExist = scalar, U08, 0, \"\", 1, 0, 0, 255, 0\n"
      "#endif\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));
  QVERIFY(!def.getConstants().contains("shouldNotExist"));
}

void ECUDefinitionTest::conditionalIfSet_keepsTrueBranch() {
  const QByteArray ini =
      "#set FEATURE_Y\n"
      "[Constants]\n"
      "page = 1\n"
      "#if FEATURE_Y\n"
      "  shouldExist = scalar, U08, 0, \"\", 1, 0, 0, 255, 0\n"
      "#endif\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));
  QVERIFY(def.getConstants().contains("shouldExist"));
}

void ECUDefinitionTest::conditionalNestedIfElse_correctBranch() {
  const QByteArray ini =
      "#set OUTER\n"
      "#unset INNER\n"
      "[Constants]\n"
      "page = 1\n"
      "#if OUTER\n"
      "  #if INNER\n"
      "    innerBranch = scalar, U08, 0, \"\", 1, 0, 0, 255, 0\n"
      "  #else\n"
      "    elseBranch = scalar, U08, 1, \"\", 1, 0, 0, 255, 0\n"
      "  #endif\n"
      "#endif\n";
  const QString path = writeFixture(ini);

  ECUDefinition def;
  QVERIFY(def.load(path));
  QVERIFY(!def.getConstants().contains("innerBranch"));
  QVERIFY(def.getConstants().contains("elseBranch"));
}

// ---------------------------------------------------------------------------
// BUG-G regression: load() must reset state.
// ---------------------------------------------------------------------------
void ECUDefinitionTest::loadResetsState_noAccumulation() {
  const QByteArray iniA =
      "[Constants]\n"
      "page = 1\n"
      "onlyInA = scalar, U08, 0, \"\", 1, 0, 0, 255, 0\n";
  const QByteArray iniB =
      "[Constants]\n"
      "page = 1\n"
      "onlyInB = scalar, U08, 0, \"\", 1, 0, 0, 255, 0\n";

  ECUDefinition def;
  QVERIFY(def.load(writeFixture(iniA)));
  QVERIFY(def.getConstants().contains("onlyInA"));

  QVERIFY(def.load(writeFixture(iniB)));
  QVERIFY(def.getConstants().contains("onlyInB"));
  // BUG-G fix: second load must NOT accumulate the first file's constants.
  QVERIFY(!def.getConstants().contains("onlyInA"));
}

// ---------------------------------------------------------------------------
// Smoke test against the real MS1 Extra INI bundled with the repo.
// If the fixture is missing (e.g. running from a stripped checkout) this
// test SKIPs rather than failing, so CI on a minimal source tree still works.
// ---------------------------------------------------------------------------
void ECUDefinitionTest::smoke_realMs1ExtraIniFixture() {
  const QString path =
      QString::fromUtf8(SOURCE_DIR_FOR_TESTS)
          + "/MS1_Extra_Example/projectCfg/mainController.ini";
  if (!QFile::exists(path)) {
    QSKIP("MS1_Extra_Example INI fixture not available in this checkout.");
  }

  ECUDefinition def;
  QVERIFY(def.load(path));
  // The MS1 Extra INI is a real-world file with hundreds of constants and a
  // populated signature — assert the parser produces nonempty results.
  QVERIFY(!def.getSignature().isEmpty());
  QVERIFY(def.getConstants().size() > 50);
  QVERIFY(def.getOutputChannels().size() > 5);
}

QTEST_APPLESS_MAIN(ECUDefinitionTest)
#include "ECUDefinitionTest.moc"
