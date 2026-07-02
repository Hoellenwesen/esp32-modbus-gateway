# Changelog

All notable security improvements and changes to this project are documented here.

## [Unreleased]

### Security Improvements

- **CSRF protection**: Added per-boot random CSRF token validated on all 5 POST endpoints (`/config`, `/reboot`, `/debug`, `/update`, `/wifi`). All forms carry a hidden `csrf` field. Validation runs before Basic Auth so forged requests return 403 without prompting for credentials.

- **OTA CSRF validation**: `POST /update` main handler now validates CSRF token, closing the one POST handler that was missing it.

- **Removed CSRF token serial leak**: The per-boot CSRF token is no longer printed to the serial console.

- **Explicit `esp_random()` include**: Added `#include <esp_system.h>` to guarantee `esp_random()` is declared, removing reliance on transitive includes (fixes potential build failure with `-Werror`).

- **Library dependency updates**:
  - `ESPAsyncWebServer` updated to `ESP32Async/ESPAsyncWebServer @ ^3.7.9` (resolved v3.11.2) — fixes CVE-2025-53094 (CRLF injection, CVSS 8.7).
  - `AsyncTCP` added as explicit dependency — picks up memory leak and stability fixes.
  - `eModbus` pinned to commit `126a95fc` (PR #435) — fixes memory leak in `ModbusClientTCPasync` that caused heap exhaustion with misbehaving TCP devices.

- **Default web password**: Changed from `"test123"` to `""` (empty). Boot-time serial warning emitted when no password is set.

- **Brute-force throttling**: Added 500ms `delay()` on failed Basic Auth in the `ADMIN_WEB_PASS` macro, limiting to ~2 attempts/second.

- **Logout endpoint**: Added `GET /logout` returning 401 with `WWW-Authenticate`, allowing the browser to forget cached credentials. Added "Logout" button on the home page.

- **Modbus debug function code restriction**: `POST /debug` validates `func ∈ {1-4}`, `slaveId ∈ [1,247]`, `count ∈ [1-125]` before executing `syncRequest`. Invalid input returns an error message.

- **XSS prevention in debug form**: Added `htmlEscape()` helper that escapes `& " < >` in all 4 user-supplied debug form values (`slaveId`, `reg`, `function`, `count`). The `WebPrint` class also HTML-escapes Modbus library debug output (`& < >`).

- **Password policy**: Minimum length of 8 and maximum of 64 characters enforced in config POST handler. Password field now uses a `placeholder` attribute instead of `value`, fixing the collision where the literal password `"****"` could never be changed.

- **Modbus bridge enable/disable toggle**: Added config checkbox to disable the Modbus TCP bridge at boot. Bridge server is skipped when unchecked, while serial RTU communication remains active.

### Fixed

- **`ADMIN_WEB_PASS` macro replaced with `adminAuth()` function**: Improves type safety and scoping. All 13 call sites updated.
- **`[&]` capture tightened to `[config]`** in OTA upload handler: Prevents potential dangling references.
- **Dead `request->redirect("/")` removed** from reboot and WiFi POST handlers — was never delivered before `ESP.restart()`.
- **`prefs.end()` added** before config-change reboot to properly close the NVS handle.

### Changed

- `src/pages.cpp`: `adminAuth()` function, `adminAuth` call sites, password length enforcement, password placeholder, bridge checkbox + POST handling, `[config]` capture, dead redirects removed
- `src/config.cpp`: `_bridgeEnabled` field + getter/setter + NVS persistence, `_webPassword` default changed from `"test123"` to `""`
- `src/main.cpp`: conditional bridge start, password-empty warning on boot, `prefs.end()` before reboot
- `include/config.h`: `_bridgeEnabled` field + getter/setter declarations
- `platformio.ini`: updated ESPAsyncWebServer, added AsyncTCP, pinned eModbus
