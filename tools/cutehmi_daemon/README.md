# CuteHMI - Daemon

Non-GUI program, which allows one to run CuteHMI project in the background.

Use `--project` command line option to specify the QML project file to open.
For example to run *CountDaemon* example use following command.

```
cutehmi_daemon --project="examples/CountDaemon/Main.qml"
```
Read system logs to investigate whether daemon is running (e.g., `journalctl -n20` on a system with *systemd*).

One may use `--app` option to tell the program to work as a foreground process (this can be useful when testing projects).

```
cutehmi_daemon --project="examples/CountDaemon/Main.qml" --app
```

You can use `--help` command line argument to see a list of all possible command line options.


## Signals

Under Unix daemon will respond to signals in a following way.

- SIGTERM causes daemon to gracefully quit with exit code set to EXIT_SUCCESS (0 on almost all systems).
- SIGINT causes daemon to gracefully quit, but exit code will be set to 128 + signal code.
- SIGQUIT causes violent termination and exits via abort.
- SIGHUP reloads the project as if daemon has been restarted.