using System.Runtime.InteropServices;
using BaoBao.Auth.Core;

namespace Veox.AuthBridge;

public static class EntryPoints
{
    private static readonly object gate = new();
    private static LicenseSession? session;
    private static int opening;

    // hostfxr's component_entry_point_fn ABI: int(void*, int32_t).
    public static int Begin(IntPtr args, int size)
    {
        if (args == IntPtr.Zero || size != sizeof(int)
            || Interlocked.Exchange(ref opening, 1) != 0) return 0;
        try
        {
            bool chinese = Marshal.ReadInt32(args) != 0;
            int result = 0;
            var ui = new Thread(() =>
            {
                LicenseSession? candidate = null;
                try
                {
                    Application.EnableVisualStyles();
                    AppIdentity.Configure(LicenseSession.AppId, LicenseSession.AppVersion);
                    candidate = new LicenseSession(new CoreLicenseService());
                    using var form = new LoginForm(candidate, chinese);
                    if (form.ShowDialog() == DialogResult.OK && candidate.IsAuthorized)
                    {
                        lock (gate) { session?.Dispose(); session = candidate; }
                        candidate.StartHeartbeat();
                        result = 1;
                        candidate = null;
                    }
                }
                catch (Exception)
                {
                    MessageBox.Show(chinese ? "网络验证组件初始化失败。" : "License component initialization failed.", "VeoxRobRun");
                }
                finally { candidate?.Dispose(); }
            });
            ui.SetApartmentState(ApartmentState.STA);
            ui.Start();
            ui.Join();
            return result;
        }
        catch { return 0; }
        finally { Volatile.Write(ref opening, 0); }
    }

    public static int Poll(IntPtr args, int size)
    {
        try { lock (gate) return session?.IsAuthorized == true ? 1 : 0; }
        catch { return 0; }
    }

    public static int End(IntPtr args, int size)
    {
        try { lock (gate) { session?.Dispose(); session = null; } return 1; }
        catch { return 0; }
    }

    // Offline ABI/dependency probe: never creates a client or grants a session.
    public static int Probe(IntPtr args, int size) =>
        typeof(LicenseClient).Assembly.GetName().Name == "BaoBao.Auth.Core" ? 1 : 0;
}
