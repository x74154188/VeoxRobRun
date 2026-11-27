using BaoBao.Auth.Core;

namespace Veox.AuthBridge;

// The SDK remains responsible for signatures, machine identity and threat checks.
// This interface permits isolated tests without production keys or HTTP requests.
internal interface ILicenseService : IDisposable
{
    Task<AuthResponse> CheckVersion();
    Task<AuthResponse> Activate(string key);
    Task<AuthResponse> Verify(string key);
    bool Validate();
    void Invalidate();
}

internal sealed class CoreLicenseService : ILicenseService
{
    private readonly LicenseClient client = LicenseClient.CreateDefault();
    public Task<AuthResponse> CheckVersion() => client.CheckVersionAsync();
    public Task<AuthResponse> Activate(string key) => client.ActivateKey(key);
    public Task<AuthResponse> Verify(string key) => client.VerifyKey(key);
    public bool Validate() => AuthSession.Validate();
    public void Invalidate() => AuthSession.Invalidate();
    public void Dispose() => client.Dispose();
}

internal sealed class LicenseSession : IDisposable
{
    internal const string AppId = "rob_run";
    internal const string AppVersion = "1.0.0";
    private readonly ILicenseService service;
    private readonly Func<long> now;
    private readonly TimeSpan heartbeat;
    private readonly TimeSpan lease;
    private readonly SemaphoreSlim operation = new(1, 1);
    private readonly CancellationTokenSource stop = new();
    private Task? worker;
    private string key = "";
    private long deadline;
    private int active, disposed;
    public string Message { get; private set; } = "";
    public string ExpiresAt { get; private set; } = "";

    public LicenseSession(ILicenseService service, Func<long>? monotonicMs = null,
        TimeSpan? heartbeatInterval = null, TimeSpan? leaseDuration = null)
    {
        this.service = service;
        now = monotonicMs ?? (() => Environment.TickCount64);
        heartbeat = heartbeatInterval ?? TimeSpan.FromSeconds(60);
        // Bounded authorization even if a network call stalls indefinitely.
        lease = leaseDuration ?? TimeSpan.FromSeconds(135);
    }

    public bool IsAuthorized
    {
        get
        {
            if (Volatile.Read(ref active) != 1 || Volatile.Read(ref disposed) != 0) return false;
            if (now() < Interlocked.Read(ref deadline)) return true;
            Revoke("授权验证超时，请重新登录。");
            return false;
        }
    }

    internal static bool VersionPermits(AuthResponse response, out string reason)
    {
        reason = response.message;
        if (!response.success) return false;
        var data = response.data;
        if (data?.app_id is { Length: > 0 } && data.app_id != AppId)
        {
            reason = "授权应用标识不匹配。";
            return false;
        }
        if (data?.version_check_enabled == true && !string.IsNullOrWhiteSpace(data.min_version))
        {
            if (!Version.TryParse(data.min_version, out var min)
                || new Version(AppVersion) < min)
            {
                reason = "当前版本低于服务器要求，请更新程序。";
                return false;
            }
        }
        return true;
    }

    public async Task<bool> LoginAsync(string input)
    {
        input = input.Trim();
        if (input.Length == 0 || input.Length > 512)
        {
            Message = "请输入有效卡密。";
            return false;
        }
        await operation.WaitAsync(stop.Token).ConfigureAwait(false);
        try
        {
            Revoke("");
            var version = await service.CheckVersion().WaitAsync(stop.Token).ConfigureAwait(false);
            if (!VersionPermits(version, out var reason)) { Message = reason; return false; }
            var response = await service.Activate(input).WaitAsync(stop.Token).ConfigureAwait(false);
            if (!VersionPermits(response, out reason) || !service.Validate())
            {
                Revoke(string.IsNullOrWhiteSpace(reason) ? "授权验证失败。" : reason);
                return false;
            }
            if (Volatile.Read(ref disposed) != 0) return false;
            key = input;
            Message = response.message;
            ExpiresAt = response.data?.expires_at ?? "";
            Interlocked.Exchange(ref deadline, now() + (long)lease.TotalMilliseconds);
            Volatile.Write(ref active, 1);
            return true;
        }
        catch (OperationCanceledException) { Revoke("验证已取消。"); return false; }
        catch (Exception) { Revoke("网络验证异常，请检查网络后重试。"); return false; }
        finally { operation.Release(); }
    }

    public void StartHeartbeat()
    {
        if (worker != null || !IsAuthorized) return;
        worker = Task.Run(async () =>
        {
            try
            {
                while (!stop.IsCancellationRequested && IsAuthorized)
                {
                    await Task.Delay(heartbeat, stop.Token).ConfigureAwait(false);
                    if (!await VerifyOnceAsync().ConfigureAwait(false)) break;
                }
            }
            catch (OperationCanceledException) { }
            catch (Exception) { Revoke("授权心跳异常，请重新登录。"); }
        });
    }

    internal async Task<bool> VerifyOnceAsync()
    {
        if (!IsAuthorized) return false;
        await operation.WaitAsync(stop.Token).ConfigureAwait(false);
        try
        {
            var response = await service.Verify(key).WaitAsync(stop.Token).ConfigureAwait(false);
            // Expired leases cannot be resurrected by a late HTTP response.
            if (!IsAuthorized) { Revoke("授权验证已过期。"); return false; }
            if (!VersionPermits(response, out var reason) || !service.Validate())
            {
                Revoke(string.IsNullOrWhiteSpace(reason) ? "授权已失效。" : reason);
                return false;
            }
            ExpiresAt = response.data?.expires_at ?? ExpiresAt;
            Interlocked.Exchange(ref deadline, now() + (long)lease.TotalMilliseconds);
            return true;
        }
        catch (OperationCanceledException) { Revoke("授权验证已停止。"); return false; }
        catch (Exception) { Revoke("授权验证失败，请重新登录。"); return false; }
        finally { operation.Release(); }
    }

    private void Revoke(string reason)
    {
        Volatile.Write(ref active, 0);
        service.Invalidate();
        Message = reason;
    }

    public void Dispose()
    {
        if (Interlocked.Exchange(ref disposed, 1) != 0) return;
        stop.Cancel();
        Revoke("");
        key = "";
        service.Dispose(); // Cancels the SDK's in-flight HttpClient requests.
        // Do not dispose the semaphore while an awaited operation is unwinding.
    }
}
