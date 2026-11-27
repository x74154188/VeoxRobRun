# BaoBao.Auth.Cpp native integration

The .NET bridge has been removed. The native overlay compiles the existing
private **BaoBao.Auth.Cpp** source directly into its x64 executable. No C# SDK,
hostfxr/nethost, CLR, .NET Desktop Runtime or `auth` runtime folder is required.
SDK source, endpoint signing secrets and binaries remain outside Git.

## Build

Copy `auth/auth.local.props.example` to the repository root as `auth.local.props`.
Set `BaoBaoAuthCppDir` to your private SDK directory. Alternatively set the
`BAOBAO_AUTH_CPP_DIR` environment variable or MSBuild property `/p:BaoBaoAuthCppDir=...`.
Build `VeoxDMA.sln` as `Release|x64` with the normal Visual Studio C++ tools.
The original SDK source is compiled unchanged with its own include directories,
no precompiled header, C++17, and the normal WinHTTP/BCrypt/WMI/DPAPI system libraries.

Use a fresh output directory when switching from the old C# build; old bridge
files are not automatically deleted from an existing directory. Distribute normal
overlay DLLs/maps/fonts as before; do not distribute the old `auth` folder.

## Behavior

- App ID `rob_run`, version `1.0.0`, defined in `Security/AuthController.h`.
- A Chinese console menu opens before DMA/device workers; there is no login dialog.
  Choose 3 to enter and activate a card, then 1 to start. The menu also offers
  online verification / expiry (2), unbind (4), exit (5 / 0), and forget (6).
- Version/app policy, activation, response signatures, machine identity and
  SDK security checks use the original C++ SDK; no compatibility reimplementation.
- The SDK sends an online heartbeat every 60 seconds. Failure revokes access.
  A 135-second monotonic lease also closes access if requests stall; a late
  success does not resurrect an expired or cancelled controller session.
- Rendering stops and aim shuts down before a normal application exit on expiry.
- Successful login or manual verification automatically remembers the card using
  SDK current-user DPAPI encryption at
  `%LOCALAPPDATA%\VeoxRobRun\rob_run\license.dat`, compatible with the earlier
  C# saved-card format. The next launch verifies that saved card online. Failure
  leaves the menu open for another login or unbind; offline startup is denied.
- The console displays the server's `expires_at` text after successful login /
  verification and refreshes it from heartbeat responses. A missing value is
  explicitly reported, never estimated. Card input is hidden and never printed.
- Unbind requires confirmation (the current server deducts three hours), calls
  the SDK, stops heartbeat and revokes local authorization even on failure.
  Success clears the corresponding saved card; unbinding a different manually
  entered card preserves the other remembered card. Forget only deletes cache.
- Console requests use the synchronous SDK. The user can choose exit at the
  menu or end console input. The SDK does not expose HTTP cancellation.
- Default English and existing Chinese UI/font features remain unchanged.

## Tests

In an x64 VS Developer shell:

```powershell
tools/auth/verify.ps1 -SdkSource <private BaoBao.Auth.Cpp directory> -OutputDirectory <new test directory>
```

Tests use the real C++ SDK response types, protocol/DPAPI implementation and
console flow, with a fake service for controller lifecycle and menu cases. No
production card, game/DMA process, remote server or security telemetry is used.
The production adapter is compiled but not instantiated by offline tests.
Live acceptance requires a valid `rob_run` card and server-side revoke test.
