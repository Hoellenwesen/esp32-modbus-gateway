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

### Changed

- `src/pages.cpp`: CSRF token generation + validation, logout route, debug input validation, XSS escaping, serial token removal, `esp_system.h` include
- `src/config.cpp`: `_webPassword` default changed from `"test123"` to `""`
- `src/main.cpp`: password-empty warning on boot
- `platformio.ini`: updated ESPAsyncWebServer, added AsyncTCP, pinned eModbus
