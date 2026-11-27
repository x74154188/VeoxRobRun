using BaoBao.Auth.Core;

namespace Veox.AuthBridge;

internal sealed class LoginForm : Form
{
    private readonly LicenseSession session;
    private readonly bool chinese;
    private readonly TextBox key = new() { UseSystemPasswordChar = true, MaxLength = 512, Dock = DockStyle.Fill };
    private readonly CheckBox remember = new() { AutoSize = true };
    private readonly Button login = new() { AutoSize = true };
    private readonly Label status = new() { AutoSize = false, Dock = DockStyle.Fill };
    private bool busy;
    private bool closed;
    private static string KeyPath => Path.Combine(Environment.GetFolderPath(
        Environment.SpecialFolder.LocalApplicationData), "VeoxRobRun", LicenseSession.AppId, "license.dat");

    public LoginForm(LicenseSession session, bool chinese)
    {
        this.session = session;
        this.chinese = chinese;
        Text = chinese ? "VeoxRobRun · 网络验证" : "VeoxRobRun · License verification";
        StartPosition = FormStartPosition.CenterScreen;
        ClientSize = new Size(560, 285);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        AutoScaleMode = AutoScaleMode.Dpi;
        Font = new Font("Microsoft YaHei UI", 10);
        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(22),
            ColumnCount = 1, RowCount = 6 };
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 32));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 38));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 34));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 40));
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 24));
        layout.Controls.Add(new Label { Text = chinese ? "请输入卡密（应用：rob_run）" : "License key (app: rob_run)", AutoSize = true });
        layout.Controls.Add(key);
        remember.Text = chinese ? "记住卡密（当前 Windows 用户加密保存）" : "Remember key (encrypted for this Windows user)";
        layout.Controls.Add(remember);
        var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill };
        login.Text = chinese ? "登录 / 激活" : "Sign in / Activate";
        var cancel = new Button { Text = chinese ? "退出" : "Exit", AutoSize = true, DialogResult = DialogResult.Cancel };
        buttons.Controls.Add(login);
        buttons.Controls.Add(cancel);
        layout.Controls.Add(buttons);
        layout.Controls.Add(status);
        layout.Controls.Add(new Label { Text = chinese ? "验证通过后才启动 DMA；运行中每 60 秒验证。" : "DMA starts after sign-in; license checked every 60 seconds.", AutoSize = true });
        Controls.Add(layout);
        AcceptButton = login;
        CancelButton = cancel;
        login.Click += async (_, _) => await LoginAsync();
        FormClosed += (_, _) => { closed = true; if (DialogResult != DialogResult.OK) session.Dispose(); };
        try
        {
            if (SecureKeyStore.TryLoad(KeyPath, out var saved)) { key.Text = saved; remember.Checked = true; }
        }
        catch { /* An unreadable saved key never grants access. */ }
    }

    private async Task LoginAsync()
    {
        if (busy) return;
        busy = true;
        login.Enabled = key.Enabled = remember.Enabled = false;
        status.Text = chinese ? "正在连接验证服务器…" : "Connecting to license server…";
        try
        {
            string input = key.Text.Trim();
            if (!await Task.Run(() => session.LoginAsync(input)))
            {
                if (!closed) status.Text = session.Message;
                return;
            }
            if (closed) return;
            try
            {
                if (remember.Checked) SecureKeyStore.Save(KeyPath, input);
                else if (File.Exists(KeyPath)) File.Delete(KeyPath);
            }
            catch
            {
                MessageBox.Show(this, chinese ? "登录成功，但保存卡密失败。" : "Signed in, but saving the key failed.", Text);
            }
            key.Clear();
            DialogResult = DialogResult.OK;
            Close();
        }
        catch (Exception)
        {
            if (!closed) status.Text = chinese ? "验证异常，请重试。" : "Verification failed. Please retry.";
        }
        finally
        {
            if (!closed) { busy = false; login.Enabled = key.Enabled = remember.Enabled = true; }
        }
    }
}
