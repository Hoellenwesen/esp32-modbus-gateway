# Changelog

All notable security improvements and changes to this project are documented here.

### 2026-07-01 — Initial Security Audit

#### Security Improvements

- **CSRF protection**: Added per-boot random CSRF token validated on all 5 POST endpoints (`/config`, `/reboot`, `/debug`, `/update`, `/wifi`). All forms carry a hidden `csrf` field. Validation runs before Basic Auth so forged requests return 403 without prompting for credentials. (Includes `POST /update` main handler that was missing CSRF validation, and removed serial leak of the token.)

- **Explicit `esp_random()` include**: Added `#include <esp_system.h>` to guarantee `esp_random()` is declared, removing reliance on transitive includes (fixes potential build failure with `-Werror`).

- **Library dependency updates**:
  - `ESPAsyncWebServer` updated to `ESP32Async/ESPAsyncWebServer @ ^3.7.9` (resolved v3.11.2) — fixes CVE-2025-53094 (CRLF injection, CVSS 8.7).
  - `AsyncTCP` added as explicit dependency — picks up memory leak and stability fixes.
  - `eModbus` pinned to commit `126a95fc` (PR #435) — fixes memory leak in `ModbusClientTCPasync` that caused heap exhaustion with misbehaving TCP devices.

- **Default web password**: Changed from `"test123"` to `""` (empty). Boot-time serial warning emitted when no password is set.

- **Brute-force throttling**: Added 500ms `delay()` on failed Basic Auth, limiting to ~2 attempts/second.

- **Logout endpoint**: Added `GET /logout` returning 401 with `WWW-Authenticate`, allowing the browser to forget cached credentials. Added "Logout" button on the home page.

- **Modbus debug function code restriction**: `POST /debug` validates `func ∈ {1-4}`, `slaveId ∈ [1,247]`, `count ∈ [1-125]` before executing `syncRequest`. Invalid input returns an error message.

- **XSS prevention in debug form**: Added `htmlEscape()` helper that escapes `& " < >` in all 4 user-supplied debug form values (`slaveId`, `reg`, `function`, `count`). The `WebPrint` class also HTML-escapes Modbus library debug output (`& < >`).

- **Password policy**: Minimum length of 8 and maximum of 64 characters enforced in config POST handler. Password field now uses a `placeholder` attribute instead of `value`, fixing the collision where the literal password `"****"` could never be changed.

- **Modbus bridge enable/disable toggle**: Added config checkbox to disable the Modbus TCP bridge at boot. Bridge server is skipped when unchecked, while serial RTU communication remains active.

- **OTA MD5 replaced with SHA256 hash verification**: Upload form and handler switched from optional MD5 to optional SHA256-256 hash verification. Incremental SHA256 computed using hardware-accelerated mbedTLS (`mbedtls/sha256.h`). Hash verified before `Update.end(true)` commits the OTA partition; mismatch triggers `Update.abort()` and returns 400.

#### Fixed

- **`ADMIN_WEB_PASS` macro replaced with `adminAuth()` function**: Improves type safety and scoping. All 13 call sites updated.
- **`[&]` capture tightened to `[config]`** in OTA upload handler: Prevents potential dangling references.
- **Dead `request->redirect("/")` removed** from reboot and WiFi POST handlers — was never delivered before `ESP.restart()`.
- **`prefs.end()` added** before config-change reboot to properly close the NVS handle.

#### Changed

- `src/pages.cpp`: `adminAuth()` function and all call sites, password length enforcement, password placeholder, bridge checkbox + POST handling, `[config]` capture, dead redirects removed, SHA256 hash verification
- `src/config.cpp`: `_bridgeEnabled` field + getter/setter + NVS persistence, `_webPassword` default changed from `"test123"` to `""`
- `src/main.cpp`: conditional bridge start, password-empty warning on boot, `prefs.end()` before reboot
- `include/config.h`: `_bridgeEnabled` field + getter/setter declarations
- `platformio.ini`: updated ESPAsyncWebServer, added AsyncTCP, pinned eModbus

### 2026-07-02 — OTA Usability & mDNS

#### Security Improvements

- **mDNS/DNS-SD network discovery**: Added configurable hostname persisted in NVS (default `"esp32-modbus-gateway"`). `MDNS.begin()` starts after WiFi connect and registers `http` service on port 80. Hostname field in config page, displayed on status page and serial telemetry.

#### Fixed

- **OTA response race condition**: Removed `request->onDisconnect([](){ ESP.restart(); })` from `POST /update` — was causing ESP to restart before the success response was delivered, leaving browser with a dropped connection ("site not reachable"). Replaced with synchronous `request->send(200, "text/html", html)` + `delay(500)` + `ESP.restart()`. Error path also guarded against NULL `beginResponse()` return.

- **OTA form field ordering**: Moved CSRF hidden input and SHA256 text input **before** the file input in the multipart form. Previously they came after the file, so the upload handler's index==0 CSRF validation always failed because the `csrf` param hadn't been parsed yet → `Update.begin()` never ran → OTA silently failed.

- **OTA progress bar spacing & text**: Added `<p></p>` between form and progress bar to fix collapsed button spacing. Removed `btn.textContent` changes so button text stays as "Upload" throughout (no duplicate status).

#### Added

- **OTA progress bar with auto-redirect**: AJAX-based upload with real-time progress stages ("Uploading... X%" → "Installing..." → "Installation complete, rebooting..."). Errors handled for 403 (session expired) and network failures. After reboot detected (polling `GET /` every 2s), auto-redirects to main page.

#### Changed

- `src/pages.cpp`: OTA main handler rewritten (sync response, removed onDisconnect), upload handler SHA256 verification, progress bar HTML/CSS/JS, form reordering, hostname field + status page display
- `src/config.cpp`: `_hostname` field + getter/setter + NVS persistence
- `src/main.cpp`: `ESPmDNS.h` include, MDNS begin + service registration, hostname in telemetry
- `include/config.h`: `_hostname` field + getter/setter declarations

### 2026-07-23 — Modbus Serial Config, Security & Reliability Fixes

#### Security Improvements

- **XSS prevention in status page**: `sendTableRow()` now HTML-escapes the `value` parameter via `htmlEscape()`. Previously `WiFi.SSID()` and other status values were injected raw into HTML, allowing stored XSS via malicious SSIDs.

- **Hostname character validation**: `Config::setHostname()` now rejects hostnames containing characters outside `[a-zA-Z0-9-]`. Prevents HTML injection in the config form and broken mDNS registration.

- **Hostname HTML escaping**: Config page hostname input field escapes the current value with `htmlEscape()` to prevent attribute injection.

- **Brute-force throttling rewritten**: Replaced blocking `delay(500)` in `adminAuth()` with a non-blocking timestamp-based throttle. Failed auth now returns HTTP 429 if attempted within 500ms of the last failure, eliminating the DoS vector where a single attacker could stall all web requests and the Modbus TCP bridge.

#### Fixed

- **Stop bits encoding off-by-one**: `encodeStopBits()` now produces the correct IDF `uart_stop_bits_t` enum values packed into bits[5:4]. Previously 1 stop bit encoded as 0x00 (invalid IDF value 0) and 2 stop bits encoded as 0x10 (1.5 stop bits). Verified against `esp32-hal-uart.c` and `uart_types.h`.

- **Stop bits decode**: `getModbusStopBits()` and `getSerialStopBits()` now return human-readable values (1 or 2) instead of raw IDF enum values (1 or 3).

- **Stop bits UI**: Removed unsupported "1.5 bits" option from both Modbus and serial stop bits dropdowns. ESP32 UART only supports 1 or 2 stop bits for data widths > 5 bits. POST handler constrained to 1-2.

- **Parity value 1 rejected**: POST handler now rejects parity value 1 (MARK), which was accepted by `constrain(0,3)` but not offered in the UI and unsupported by the ESP32 UART config.

- **OTA auto-redirect after reboot**: Extracted polling logic into shared `startPoll()` function, now called from both the XHR `load` handler (status 200) and the `error` handler (when upload progress was already at 100%). Previously if the XHR response was lost during `delay(500)` + `ESP.restart()`, the `error` handler displayed "Installation complete, rebooting..." but never started the redirect loop.

- **OTA auth/CSRF blocking on every chunk**: Moved auth and CSRF validation to run only at `index == 0` (first chunk) with an `otaAuthenticated` flag guarding subsequent chunks. Previously `adminAuth()` with its 500ms throttle ran on every upload chunk, making OTA uploads extremely slow.

- **`MODBUS_QUEUE_DEPTH` misuse**: Renamed to `RTU_CORE_ID`. The value `1` was being passed as the `coreID` parameter to `ModbusClientRTU::begin(HardwareSerial&, int)`, not as a queue depth. Actual queue depth was silently 100 (constructor default). The core1 pinning is correct and retained.

- **`configChanged` data race**: Changed from `bool` to `volatile bool` since it is written from an async web server handler and read from `loop()` on different tasks.

- **Debug serial reinit logging**: Added `dbgln("[serial] switching to configured baud rate")` before `debugSerial.end()` so the baud rate switch is visible in serial output.

#### Changed

- `src/config.cpp`: `encodeStopBits()` IDF enum fix, `getModbusStopBits()`/`getSerialStopBits()` decode fix, `setHostname()` character validation
- `src/pages.cpp`: Stop bits/parity constraints, `sendTableRow()` XSS fix, `htmlEscape()` repositioned, hostname escaping, `adminAuth()` non-blocking throttle, OTA auth at index==0, `startPoll()` for auto-redirect
- `src/main.cpp`: `volatile bool configChanged`, `RTU_CORE_ID` usage, serial reinit log
- `include/config.h`: `volatile bool configChanged`, `RTU_CORE_ID` define
- `AGENTS.md`: Created with build commands, architecture, library quirks, and gotchas
