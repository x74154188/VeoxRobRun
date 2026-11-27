using BaoBao.Auth.Core;
using Veox.AuthBridge;

int checks = 0, failed = 0;
void Check(bool value, string label)
{
    checks++;
    if (!value) { failed++; Console.WriteLine("FAIL: " + label); }
}
AuthResponse Ok() => new() { success = true, data = new() { app_id = "rob_run", expires_at = "2099-01-01" } };

Check(LicenseSession.AppId == "rob_run", "requested application identity");
Check(!LicenseSession.VersionPermits(new() { success = false }, out _), "version rejection");
Check(!LicenseSession.VersionPermits(new() { success = true, data = new() { app_id = "another-app" } }, out _), "cross-app response rejected");
Check(!LicenseSession.VersionPermits(new() { success = true, data = new() { version_check_enabled = true, min_version = "999.0.0" } }, out _), "required update rejected");
Check(!LicenseSession.VersionPermits(new() { success = true, data = new() { version_check_enabled = true, min_version = "invalid" } }, out _), "malformed version denied");
Check(LicenseSession.VersionPermits(Ok(), out _), "normal signed SDK result accepted by policy");

long clock = 0;
var fake = new FakeService { Response = Ok() };
using (var session = new LicenseSession(fake, () => clock))
{
    Check(!session.IsAuthorized, "default deny");
    Check(!await session.LoginAsync(" "), "empty key rejected");
    Check(!await session.LoginAsync(new string('x', 513)), "oversized key rejected");
    Check(fake.Calls == 0, "invalid input makes no service calls");
    Check(await session.LoginAsync(" TEST-CARD "), "successful activation");
    Check(fake.LastKey == "TEST-CARD", "key whitespace normalized");
    Check(session.IsAuthorized, "verified login authorizes");
    Check(session.ExpiresAt == "2099-01-01", "expiration returned");
    clock = 60000;
    Check(await session.VerifyOnceAsync(), "heartbeat success");
    clock = 140000;
    Check(session.IsAuthorized, "heartbeat renewed monotonic lease");
    fake.Response = new() { success = false, message = "revoked" };
    Check(!await session.VerifyOnceAsync(), "revoked heartbeat rejected");
    Check(!session.IsAuthorized && !fake.Valid, "revoked session invalidated");
    fake.Response = Ok();
    Check(!await session.VerifyOnceAsync(), "heartbeat cannot revive revoked session");
}
Check(fake.Disposed, "SDK service disposed");

fake = new FakeService { Response = Ok(), VersionResponse = new() { success = false } };
using (var session = new LicenseSession(fake))
{
    Check(!await session.LoginAsync("TEST"), "server version failure prevents login");
    Check(fake.Activations == 0 && !session.IsAuthorized, "no activation after version refusal");
}
fake = new FakeService { Response = new() { success = false } };
using (var session = new LicenseSession(fake))
    Check(!await session.LoginAsync("TEST") && !session.IsAuthorized, "activation failure denied");
fake = new FakeService { Response = Ok(), PermitSession = false };
using (var session = new LicenseSession(fake))
    Check(!await session.LoginAsync("TEST") && !session.IsAuthorized, "SDK session validation required");
fake = new FakeService { Throw = true };
using (var session = new LicenseSession(fake))
    Check(!await session.LoginAsync("TEST") && !session.IsAuthorized, "network exception denied");

clock = 0;
fake = new FakeService { Response = Ok() };
using (var session = new LicenseSession(fake, () => clock))
{
    Check(await session.LoginAsync("TEST"), "lease setup");
    clock = 135000;
    Check(!session.IsAuthorized, "lease exact timeout denied");
    clock = 0;
    Check(!session.IsAuthorized, "clock rollback does not resurrect revoked authorization");
}

clock = 0;
fake = new FakeService { Response = Ok() };
using (var session = new LicenseSession(fake, () => clock))
{
    Check(await session.LoginAsync("TEST"), "late response setup");
    var reply = new TaskCompletionSource<AuthResponse>(TaskCreationOptions.RunContinuationsAsynchronously);
    fake.PendingVerify = reply.Task;
    var pending = session.VerifyOnceAsync();
    clock = 135000;
    Check(!session.IsAuthorized, "stalled heartbeat lease expires");
    reply.SetResult(Ok());
    Check(!await pending && !session.IsAuthorized, "late success cannot resurrect access");
}

fake = new FakeService { Response = Ok() };
var canceled = new LicenseSession(fake);
Check(await canceled.LoginAsync("TEST"), "cancel setup");
fake.PendingVerify = new TaskCompletionSource<AuthResponse>().Task;
var inFlight = canceled.VerifyOnceAsync();
canceled.Dispose();
Check(!await inFlight && !canceled.IsAuthorized, "in-flight cancellation denied");
canceled.Dispose();
Check(fake.Disposed, "repeated disposal harmless");

fake = new FakeService { Response = Ok() };
using (var session = new LicenseSession(fake, heartbeatInterval: TimeSpan.FromMilliseconds(15)))
{
    Check(await session.LoginAsync("TEST"), "worker setup");
    fake.Response = new() { success = false };
    session.StartHeartbeat();
    var until = DateTime.UtcNow.AddSeconds(3);
    while (session.IsAuthorized && DateTime.UtcNow < until) await Task.Delay(10);
    Check(!session.IsAuthorized && fake.Verifies > 0, "automatic heartbeat revokes");
}

var path = Path.Combine(AppContext.BaseDirectory, "字体测试", "license.dat");
SecureKeyStore.Save(path, "TEST-NOT-A-PRODUCTION-CARD");
Check(SecureKeyStore.TryLoad(path, out var saved) && saved == "TEST-NOT-A-PRODUCTION-CARD", "DPAPI round trip");
Check(!System.Text.Encoding.UTF8.GetString(File.ReadAllBytes(path)).Contains("TEST-NOT-A-PRODUCTION-CARD"), "saved card not plaintext");
Check(EntryPoints.Probe(IntPtr.Zero, 0) == 1, "real SDK assembly dependency loaded");
Check(EntryPoints.Poll(IntPtr.Zero, 0) == 0, "bridge default deny");
Check(EntryPoints.Begin(IntPtr.Zero, 0) == 0, "invalid ABI request denied without UI");
Check(EntryPoints.End(IntPtr.Zero, 0) == 1 && EntryPoints.Poll(IntPtr.Zero, 0) == 0, "bridge shutdown denies access");
Console.WriteLine($"{(failed == 0 ? "PASS" : "FAIL")}: auth controller; {checks} checks; {failed} failures");
return failed == 0 ? 0 : 1;

sealed class FakeService : ILicenseService
{
    public AuthResponse Response = new() { success = true };
    public AuthResponse VersionResponse = new() { success = true };
    public bool PermitSession = true, Valid, Throw, Disposed;
    public int Calls, Activations, Verifies;
    public string LastKey = "";
    public Task<AuthResponse>? PendingVerify;
    public Task<AuthResponse> CheckVersion()
    {
        Calls++;
        if (Throw) throw new HttpRequestException();
        return Task.FromResult(VersionResponse);
    }
    public Task<AuthResponse> Activate(string key)
    {
        Calls++; Activations++; LastKey = key;
        Valid = Response.success && PermitSession;
        return Task.FromResult(Response);
    }
    public Task<AuthResponse> Verify(string key)
    {
        Calls++; Verifies++;
        Valid = Response.success && PermitSession;
        return PendingVerify ?? Task.FromResult(Response);
    }
    public bool Validate() => Valid && PermitSession;
    public void Invalidate() => Valid = false;
    public void Dispose() => Disposed = true;
}
