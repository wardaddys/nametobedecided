# OS Tuner — Tests, Build, and Documentation Review

**Date:** June 1, 2026
**Commit Hash:** `e103ed9`

## EXECUTIVE SUMMARY

The project has made excellent progress from 1 test file (11 cases) to 6 test files (~50 cases), covering the protocol layer, INI parser, serial handshake, burn chain, MSQ round-trip, and guardrails. Tests are well-written, well-documented, and target the right things. However, there are significant repository hygiene issues — the `.gitignore` is actively failing to exclude committed build artifacts, and the repo contains ~500KB of junk files plus a ~499KB nonsense `.test` file. Eight internal planning documents totaling ~157KB are checked in and would be inappropriate for a public GitHub repo.

---

## 1. TEST SUITE REVIEW

### 1.1 SpeeduinoProtocolTest.cpp — EXCELLENT (281 lines, 16 tests)
**Verdict: Best test file in the project.**
- **Strengths:** CRC32 known-answer vectors, frame wrap/unwrap with corrupt CRC rejection, truncation rejection, length mismatch rejection. Short-payload safety tested. Signature parser + ACK/NAK detection covered.
- **Issues:** `crc32_realPagePayload` doesn't compare against a known external CRC value for the 0x00-0xFF payload. 

### 1.2 ECUDefinitionTest.cpp — VERY GOOD (298 lines, 11 tests)
**Verdict: Thorough parser lockdown.**
- **Strengths:** Inline fixture strings, tests all five parsed sections, conditional parsing tested, tests state accumulation reset (BUG-G regression).
- **Issues:** `writeFixture` creates `QTemporaryFile` on heap without immediate deletion on Windows. No test for malformed INI input.

### 1.3 SerialHandshakeTest.cpp — GOOD (119 lines, 6 tests)
**Verdict: Good simulation-path coverage, but limited by architecture.**
- **Strengths:** Simulation mode lifecycle tested, signature extraction verified.
- **Issues:** `signatureGate_blocksWriteBeforeValidation` doesn't test the gate. No test for reconnection behavior or disconnection.

### 1.4 BurnChainTest.cpp — ADEQUATE (88 lines, 5 tests)
**Verdict: Locks the public contract but can't test the actual chain.**
- **Strengths:** Documents the BUG-A data-loss bug thoroughly.
- **Issues:** The actual burn chain (marking pages dirty, burning sequentially, verifying each) is **untested**. This is a contract lock, not a functional test.

### 1.5 MsqRoundTripTest.cpp — ADEQUATE (107 lines, 5 tests)
**Verdict: Good negative-path coverage, but the "round-trip" in the name is aspirational.**
- **Issues:** There is **no actual round-trip test**. The save-path tests only verify rejection on invalid inputs. Uses hardcoded path `/tmp/should_not_exist.msq` which may fail on Windows.

### 1.6 ECUSettingsManagerGuardrailsTest.cpp — GOOD (232 lines, 11 tests)
**Verdict: Solid bounds validation coverage.**
- **Issues:** Uses `QTEST_MAIN` instead of `QTEST_APPLESS_MAIN`. Windows line endings (CRLF).

---

## 2. BUILD SYSTEM (CMakeLists.txt)
**Verdict: Well-structured, a few issues.**
- **High:** Version mismatch. CMakeLists says `VERSION 0.5.0`, version.json says `"0.5.0-alpha"`.
- **Medium:** Install target is incomplete. `RUNTIME DESTINATION .` installs only the .exe. No `windeployqt` step. Test executables link against real SerialManager/SpeeduinoProtocol pulling `QSerialPort` into all tests.

---

## 3. .gitignore — CRITICAL ISSUES
**Verdict: The .gitignore is contradictory and failing.**
The `.gitignore` has `*.txt`, `*.log`, `compile_commands.json`, and `erros.md` listed, but the root directory currently contains **all of these files** tracked in git.
**Total waste in repo: ~2.3 MB of files that serve no purpose.**

---

## 4. DashboardWidget.cpp.test — MUST DELETE
**Verdict: 499KB junk file. Delete immediately.**
This is a **10,002-line file** consisting entirely of auto-generated filler. It contains zero test logic and adds 499KB of bloat to every clone.

---

## 5. README.md & LICENSE
- **README:** Good update, but minimum Qt version is listed as 6.5+ while other docs say 6.10.2. 
- **LICENSE:** Legally inadequate. It's 8 lines of vague pseudo-BSD text with no standard license identifier. This file does NOT grant any license to users.

---

## 6. Planning/Internal Documents
**Verdict: 8 internal docs totaling ~157KB. Inappropriate for a public repo.**
Documents like `os_tuner_audit_2026-05-13.md` document exact auto-updater RCE vulnerabilities. Publishing this before all fixes ship tells attackers exactly where to look. They should be moved to a `docs/internal/` directory that's gitignored.

---

## 7. SUMMARY OF ACTIONS

### Critical (do immediately)
1. **Delete** `src/widgets/DashboardWidget.cpp.test` — 499KB junk
2. **`git rm --cached`** all 16+ build artifact files in root
3. **Move or delete** `os_tuner_audit_2026-05-13.md` — security risk if repo goes public
4. **Fix LICENSE** — current file grants no rights

### High (before next release)
5. **Add a real MSQ round-trip test** to `MsqRoundTripTest.cpp`
6. **Fix `/tmp/` paths** in MsqRoundTripTest — won't work on Windows
7. **Add burn-chain functional test** (even with mocked serial)
8. **Move planning docs** to `.gitignore`d directory or separate repo
9. **Delete** `build_test.py`, `run_build.py`, `run.ps1` — machine-specific scratch scripts
