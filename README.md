# persistent-touch-id-sudo

Configures PAM on macOS via a Launch Daemon so that Touch ID for `sudo` is always available
and persists across OS upgrades.

## How It Works

The project installs two files on your system:

1. Executable: `/usr/local/bin/persistent-touch-id-sudo`
2. Launch daemon: `/Library/LaunchDaemons/com.yuriyguts.persistent-touch-id-sudo.plist`

When macOS starts up, it runs the launch daemon, which defines how to run the executable
and where to store its logs.

The executable checks if Touch ID is already configured in `/etc/pam.d/sudo`.
If it's not, it adds a new configuration line there allowing Touch ID to be used.

### Why is it a C binary instead of a shell script?

Full Disk Access permissions cannot be granted to individual shell scripts. While we can
add them to the allow list, it will only work if we allow full disk access for all
`env` or `bash` processes, which is too permissive. Building the tool as a standalone
native binary allows adding it to the allow list directly.

## Installing

You'll need CMake to build and install this project.

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

Then, go to Preferences > Security & Privacy > Full Disk Access, and add
the executable `/usr/local/bin/persistent-touch-id-sudo` to the allow list.

Restart macOS in order for the launch daemon to take effect.

## Uninstalling

```shell
$ sudo rm /Library/LaunchDaemons/com.yuriyguts.persistent-touch-id-sudo.plist
$ sudo rm /usr/local/bin/persistent-touch-id-sudo
```

Then, go to Preferences > Security & Privacy > Full Disk Access, and remove
`/usr/local/bin/com.yuriyguts.persistent-touch-id-sudo` from the allow list.

If you'd like to stop using Touch ID for sudo as well, edit `/etc/pam.d/sudo` manually
and remove the line containing `pam_tid.so`.

## Troubleshooting

If `sudo` still prompts you for the password, check the log files:
```shell
$ cat /tmp/com.yuriyguts.persistent-touch-id-sudo.stdout.log
$ cat /tmp/com.yuriyguts.persistent-touch-id-sudo.stderr.log
```
