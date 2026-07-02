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
