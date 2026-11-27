# .NET native hosting ABI headers

`nethost.h`, `hostfxr.h` and `coreclr_delegates.h` are unchanged copies from
the installed Microsoft.NETCore.App.Host.win-x64 9.0.20 package. `LICENSE.txt`
is the matching dotnet/runtime v9.0.20 MIT license. They define the hosting ABI;
Veox does not duplicate private CLR structures or link to a machine-specific SDK path.

The matching `nethost.dll` is staged at build time, not stored in Git.
Source: https://github.com/dotnet/runtime/tree/v9.0.20/src/native/corehost
Hosting guide: https://learn.microsoft.com/dotnet/core/tutorials/netcore-hosting
