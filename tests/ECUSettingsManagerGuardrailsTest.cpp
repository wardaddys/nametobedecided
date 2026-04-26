#include <QtTest/QtTest>

#include "core/ECUSettingsManager.h"

class ECUSettingsManagerGuardrailsTest : public QObject {
  Q_OBJECT

private slots:
  void validateConstantBounds_acceptsValidScalar();
  void validateConstantBounds_rejectsInvalidPage();
  void validateConstantBounds_rejectsUnsupportedType();
  void validateConstantBounds_rejectsOutOfBoundsOffset();
  void validateConstantBounds_rejectsInvalidBitRange();
  void validateConstantCollisions_acceptsAdjacentConstants();
  void validateConstantCollisions_rejectsOverlappingConstants();

  void validateTableBounds_acceptsValidTable();
  void validateTableBounds_rejectsInvalidDimensions();
  void validateTableBounds_rejectsInvalidElementSize();
  void validateTableBounds_rejectsZeroScale();
  void validateTableBounds_rejectsOutOfBoundsAddress();
};

void ECUSettingsManagerGuardrailsTest::validateConstantBounds_acceptsValidScalar() {
  ECUSettingsManager mgr;
  ECUDefinition::Constant def;
  def.name = "validScalar";
  def.paramClass = "scalar";
  def.type = "U16";
  def.page = 1;
  def.offset = 8;

  QString error;
  QVERIFY(mgr.validateConstantBounds(def, &error));
  QVERIFY(error.isEmpty());
}

void ECUSettingsManagerGuardrailsTest::validateConstantBounds_rejectsInvalidPage() {
  ECUSettingsManager mgr;
  ECUDefinition::Constant def;
  def.name = "badPage";
  def.paramClass = "scalar";
  def.type = "U08";
  def.page = 0;
  def.offset = 0;

  QString error;
  QVERIFY(!mgr.validateConstantBounds(def, &error));
  QVERIFY(error.contains("Invalid page"));
}

void ECUSettingsManagerGuardrailsTest::validateConstantBounds_rejectsUnsupportedType() {
  ECUSettingsManager mgr;
  ECUDefinition::Constant def;
  def.name = "badType";
  def.paramClass = "scalar";
  def.type = "U64";
  def.page = 1;
  def.offset = 0;

  QString error;
  QVERIFY(!mgr.validateConstantBounds(def, &error));
  QVERIFY(error.contains("Unsupported type"));
}

void ECUSettingsManagerGuardrailsTest::validateConstantBounds_rejectsOutOfBoundsOffset() {
  ECUSettingsManager mgr;
  ECUDefinition::Constant def;
  def.name = "overflow";
  def.paramClass = "scalar";
  def.type = "U16";
  def.page = 4;
  def.offset = 127; // page 4 fallback size is 128, U16 needs 2 bytes

  QString error;
  QVERIFY(!mgr.validateConstantBounds(def, &error));
  QVERIFY(error.contains("Out-of-bounds"));
}

void ECUSettingsManagerGuardrailsTest::validateConstantBounds_rejectsInvalidBitRange() {
  ECUSettingsManager mgr;
  ECUDefinition::Constant def;
  def.name = "badBits";
  def.paramClass = "bits";
  def.type = "U08";
  def.page = 1;
  def.offset = 8;
  def.bitField.lowBit = 6;
  def.bitField.highBit = 2;

  QString error;
  QVERIFY(!mgr.validateConstantBounds(def, &error));
  QVERIFY(error.contains("Invalid bit range"));
}

void ECUSettingsManagerGuardrailsTest::validateConstantCollisions_acceptsAdjacentConstants() {
  ECUSettingsManager mgr;

  ECUDefinition::Constant first;
  first.name = "first";
  first.paramClass = "scalar";
  first.type = "U16";
  first.page = 2;
  first.offset = 10;

  ECUDefinition::Constant second;
  second.name = "second";
  second.paramClass = "scalar";
  second.type = "U08";
  second.page = 2;
  second.offset = 12; // Adjacent to first [10..11]

  QMap<QString, ECUDefinition::Constant> defs;
  defs.insert(first.name, first);
  defs.insert(second.name, second);

  QString error;
  QVERIFY(mgr.validateConstantCollisions(defs, &error));
  QVERIFY(error.isEmpty());
}

void ECUSettingsManagerGuardrailsTest::validateConstantCollisions_rejectsOverlappingConstants() {
  ECUSettingsManager mgr;

  ECUDefinition::Constant base;
  base.name = "base";
  base.paramClass = "scalar";
  base.type = "U16";
  base.page = 3;
  base.offset = 20;

  ECUDefinition::Constant overlap;
  overlap.name = "overlap";
  overlap.paramClass = "scalar";
  overlap.type = "U16";
  overlap.page = 3;
  overlap.offset = 21; // Overlaps base [20..21]

  QMap<QString, ECUDefinition::Constant> defs;
  defs.insert(base.name, base);
  defs.insert(overlap.name, overlap);

  QString error;
  QVERIFY(!mgr.validateConstantCollisions(defs, &error));
  QVERIFY(error.contains("collision", Qt::CaseInsensitive));
  QVERIFY(error.contains("page 3", Qt::CaseInsensitive));
}

void ECUSettingsManagerGuardrailsTest::validateTableBounds_acceptsValidTable() {
  ECUSettingsManager mgr;
  ECUDefinition::Table table;
  table.name = "validTable";
  table.page = 8;   // fallback size 384
  table.address = 64;
  table.rows = 16;
  table.cols = 16;
  table.elementSize = 1;
  table.scale = 1.0;
  table.translate = 0.0;

  QString error;
  QVERIFY(mgr.validateTableBounds(table, &error));
  QVERIFY(error.isEmpty());
}

void ECUSettingsManagerGuardrailsTest::validateTableBounds_rejectsInvalidDimensions() {
  ECUSettingsManager mgr;
  ECUDefinition::Table table;
  table.name = "badDims";
  table.page = 8;
  table.address = 0;
  table.rows = 0;
  table.cols = 16;
  table.elementSize = 1;
  table.scale = 1.0;

  QString error;
  QVERIFY(!mgr.validateTableBounds(table, &error));
  QVERIFY(error.contains("Invalid table dimensions"));
}

void ECUSettingsManagerGuardrailsTest::validateTableBounds_rejectsInvalidElementSize() {
  ECUSettingsManager mgr;
  ECUDefinition::Table table;
  table.name = "badElem";
  table.page = 8;
  table.address = 0;
  table.rows = 8;
  table.cols = 8;
  table.elementSize = 4;
  table.scale = 1.0;

  QString error;
  QVERIFY(!mgr.validateTableBounds(table, &error));
  QVERIFY(error.contains("Invalid table element size"));
}

void ECUSettingsManagerGuardrailsTest::validateTableBounds_rejectsZeroScale() {
  ECUSettingsManager mgr;
  ECUDefinition::Table table;
  table.name = "badScale";
  table.page = 8;
  table.address = 0;
  table.rows = 8;
  table.cols = 8;
  table.elementSize = 1;
  table.scale = 0.0;

  QString error;
  QVERIFY(!mgr.validateTableBounds(table, &error));
  QVERIFY(error.contains("Invalid table scale"));
}

void ECUSettingsManagerGuardrailsTest::validateTableBounds_rejectsOutOfBoundsAddress() {
  ECUSettingsManager mgr;
  ECUDefinition::Table table;
  table.name = "overflowTable";
  table.page = 4;   // fallback size 128
  table.address = 120;
  table.rows = 4;
  table.cols = 4;
  table.elementSize = 2; // 32 bytes total, exceeds page size
  table.scale = 1.0;

  QString error;
  QVERIFY(!mgr.validateTableBounds(table, &error));
  QVERIFY(error.contains("Out-of-bounds table"));
}

QTEST_MAIN(ECUSettingsManagerGuardrailsTest)
#include "ECUSettingsManagerGuardrailsTest.moc"
