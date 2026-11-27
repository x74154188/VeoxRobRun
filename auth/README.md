# BaoBao.Auth.Core integration

The native overlay hosts the original C# SDK in-process using Microsoft's
`nethost` / `hostfxr` ABI. SDK sources remain external and unmodified: the local
project links their top-level `.cs` files, with the same NuGet dependencies.
No SDK secret, license key, private source or compiled SDK binary belongs in Git.
This build does not invoke the original SDK's application-specific Obfuscar target.

## Behavior

- Application identity: `rob_run`, client version `1.0.0` (in `LicenseSession.cs`).
- A bilingual sign-in window opens before DMA/game/device workers start.
- Sign-in checks server application/version policy, activates/verifies a key via
  the existing SDK, and requires a valid SDK session. Server responses are still
  signature-checked by the original SDK.
- Heartbeat verifies the key every 60 seconds. A rejection, malformed response,
  exception or 135-second monotonic authorization lease timeout revokes access.
  Late responses cannot resurrect a revoked session. The native render tick
  stops aim, suppresses drawing and requests normal shutdown after revocation.
- Remembering a key uses the SDK's current-Windows-user DPAPI store at
  `%LOCALAPPDATA%\VeoxRobRun\rob_run\license.dat`; it never grants offline access.
- Cancel, missing bridge/runtime, invalid version, and any sign-in error never
  start DMA. A saved key is prefilled, not automatically submitted.
- SDK security checks remain unchanged. Production endpoint defaults are not
  replaced. Server messages may retain the server's own language.

## Build

Prerequisites: Visual Studio C++ x64 tools, .NET 9 SDK, NuGet access.
Copy `auth/auth.local.props.example` to the repository root as
`auth.local.props`, then point `BaoBaoAuthCoreDir` at your private SDK directory.
Alternatively set environment variable `BAOBAO_AUTH_CORE_DIR`, or pass
`/p:BaoBaoAuthCoreDir=...` to MSBuild. The local props file is ignored by Git.

Build `VeoxDMA.sln` as `Release|x64`. The native project automatically publishes
the bridge and SDK into `<OutDir>\auth\` after building.
Distribute that entire `auth` folder beside the executable, not just the EXE.
End users need the **x64 .NET 9 Windows Desktop Runtime**.
Keep normal overlay dependencies and assets beside the EXE as before.

The current runtime family matches the supplied SDK. Updating its runtime target
and retesting SDK dependencies is a separate maintenance change.

## Offline tests

`tools/auth/verify.ps1 -CoreSource <private SDK path> -OutputDirectory <new test folder>`
builds isolated controller tests using a fake service and real SDK response types,
then a native host harness against the real bridge. It checks rejection,
activation, heartbeat, cancellation, expiry, saved-key DPAPI round trips, ABI
loading, and default-deny. It sends no production HTTP requests or telemetry,
opens no game or DMA device, and creates no production license.

Manual acceptance requires a valid `rob_run` card: sign in, confirm DMA starts,
then revoke/expire the card and confirm the program exits on heartbeat failure.
