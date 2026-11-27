"""Run console input tests against a private, hidden console, not the user's UI."""
import subprocess
import sys

startup = subprocess.STARTUPINFO()
startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = subprocess.SW_HIDE
result = subprocess.run(
    [sys.argv[1]], creationflags=subprocess.CREATE_NEW_CONSOLE, startupinfo=startup,
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60,
)
sys.stdout.write(result.stdout.decode("utf-8", errors="replace"))
sys.exit(result.returncode)
