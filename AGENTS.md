# AI Instructions and Project Preferences for qgvdial

This file defines the project-specific rules, build procedures, and coding guidelines for AI assistants working in this repository.

---

## 1. Build and Compilation Rules

### Default Build Command
* **Mandatory Rule**: The compilation and build step MUST always use the default build command specified in the VS Code configuration ([.vscode/tasks.json](file:///.vscode/tasks.json)).
* To determine the build command:
  1. Inspect [.vscode/tasks.json](file:///.vscode/tasks.json) for the task where `"group": { "kind": "build", "isDefault": true }`.
  2. Run the exact command configured (currently: `make qgvdial_ubuntu_x86_64_ctr`).
* **Do not** run arbitrary ad-hoc compiler commands or alternative make targets unless explicitly instructed by the user.

---

## 2. Project Architecture & Environment

* **Target Application**: `qgvdial` (Google Voice desktop client written in C++ and Qt).
* **Qt Version**: Qt 6.8.2.
* **Build Environment**: Build runs inside a container via Docker/container commands defined in the [Makefile](file:///Makefile) (`make qgvdial_ubuntu_x86_64_ctr`).

---

## 3. Qt 6 Development & Migration Guidelines

* **Signal & Slot Syntax**:
  * Use modern pointer-to-member-function `connect()` syntax (`connect(sender, &Sender::signal, receiver, &Receiver::slot)`).
  * Ensure slot parameter types and counts are compatible with the signal. For example, `QAction::triggered(bool checked = false)` passes a boolean; slots with no arguments must match the signature overload or use a lambda if converting between incompatible signatures.
* **Modernized Qt APIs**:
  * **Regex**: Use `QRegularExpression` and `QRegularExpressionMatch` instead of deprecated/removed `QRegExp`.
  * **XML**: Use `QXmlStreamReader` instead of removed SAX classes (`QXmlDefaultHandler`, `QXmlSimpleReader`, etc.).
  * **Timestamps**: Use `QDateTime::fromSecsSinceEpoch` / `toSecsSinceEpoch` instead of `fromTime_t` / `toTime_t`.
  * **State Machine**: Include headers from the `QtStateMachine` module (`#include <QStateMachine>`, `#include <QState>`, `#include <QFinalState>`).
* **Code Integrity**:
  * Preserve existing comments, license headers, and architecture conventions.
  * Follow established coding styles in neighboring files.
