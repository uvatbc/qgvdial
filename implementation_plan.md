# Qt5 to Qt6 Compilation Migration Plan

This plan addresses the compilation errors encountered when building `qgvdial` with Qt 6.8.2 in the Ubuntu container (`accupara/qgvdial:qt6`).

## Background & Analysis

In Qt 6, several deprecated Qt 5 APIs were removed:
1. **SAX XML Classes Removed from QtXml**: `QXmlDefaultHandler`, `QXmlSimpleReader`, `QXmlInputSource`, `QXmlAttributes` were completely removed. `QtCore5Compat` is not installed in the build container.
2. **`QRegExp` Removed**: Replaced by `QRegularExpression`.
3. **`QDateTime::fromTime_t` and `toTime_t` Removed**: Replaced by `fromSecsSinceEpoch` and `toSecsSinceEpoch`.
4. **`QStateMachine` Moved**: Moved from `QtCore` to the `QtStateMachine` module and requires explicit `#include <QStateMachine>`, `#include <QState>`, `#include <QFinalState>`.
5. **O2 `setClientEmailHint` Removed**: The updated third-party `o2` library replaced custom hint properties with `extraRequestParams()`.
6. **`toTime_t` in third-party O2**: `o1.cpp`, `o1requestor.cpp`, and `oxtwitter.cpp` still call `toTime_t()`.

## Proposed Changes

### 1. XML SAX to QXmlStreamReader Migration

#### `HtmlFieldParser.h` & `HtmlFieldParser.cpp`
- Remove inheritance from `QXmlDefaultHandler`.
- Provide `bool parse(const QString &xmlData);`.
- Use `QXmlStreamReader` to collect elements into `elems` and element attributes into `attrMap`.

#### `ContactsXmlHandler.h` & `ContactsXmlHandler.cpp`
- Remove `QXmlDefaultHandler` base class.
- Provide `bool parse(const QByteArray &xmlData);`.
- Use `QXmlStreamReader` to stream through elements, emit `oneContact(ContactInfo)` and `status(...)`, and maintain contact count metrics.

#### `ContactsParser.cpp`
- Remove `QXmlSimpleReader` and `QXmlInputSource`.
- Connect signals from `ContactsXmlHandler` and call `contactsHandler.parse(byData)`.

#### `GVApi.cpp` & `GVApi_login.cpp`
- Replace `QXmlSimpleReader` and `QXmlInputSource` calls with `xmlHandler.parse(...)`.

---

### 2. State Machine Headers

#### `GVApi_login.h`
- Add `#include <QStateMachine>`, `#include <QState>`, `#include <QFinalState>`.

---

### 3. QRegExp to QRegularExpression

#### `GVApi.cpp`
- Replace `QRegExp("^\\d*$")` with `QRegularExpression("^\\d*$")`.
- Replace `strTemp.remove(QRegExp("\\d"))` with `strTemp.remove(QRegularExpression("\\d"))`.
- In `fixAmpersandEncoded`: update to `QRegularExpression rx("&#(.*?);")` and `rx.match(...)`.

#### `GVApi_login.cpp`
- In `parseFormFields`: update `rx1` to `QRegularExpression rx1("<input(.*?)>")` and `rx1.globalMatch(...)`.
- In `parseForm`: update `rxForm` to `QRegularExpression rxForm("<form(.*?)>")` and `rxForm.match(...)`.

#### `qtlocalpeer.cpp`
- Replace `QRegExp` with `QRegularExpression`.

---

### 4. QDateTime `fromTime_t` and `toTime_t` Migration

#### `GVApi.cpp`
- Replace `QDateTime::fromTime_t(...)` with `QDateTime::fromSecsSinceEpoch(...)` on lines 1031, 1145, 2503, and 2565.

#### `third-party/o2`
- Replace `toTime_t()` with `toSecsSinceEpoch()` in `o1.cpp`, `o1requestor.cpp`, and `oxtwitter.cpp`.

---

### 5. O2 `setClientEmailHint` Migration

#### `GContactsApi.cpp`
- Replace `m_o2->setClientEmailHint(task->inParams["user"].toString());` with:
  ```cpp
  QVariantMap extraParams = m_o2->extraRequestParams();
  extraParams.insert("login_hint", task->inParams["user"].toString());
  m_o2->setExtraRequestParams(extraParams);
  ```

---

## Verification Plan

### Automated Build Verification
- Run the default build task from `.vscode/tasks.json`:
  ```bash
  make qgvdial_ubuntu_x86_64_ctr
  ```
- Iterate on any subsequent compiler or linker errors until the compilation succeeds.
