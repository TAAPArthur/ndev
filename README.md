# ndev nano sized device manager

# Prereqs
Compile kernel with `CONFIG_UEVENT_HELPER`

# Install
`make install`

# Features

* mdev/sdev like syntax
* Run commands on device add, remove or both
* Auto create device nodes and change ownerships and permission
* Load modules via modprobe out of the box
* Works with (libudev-zero)[https://github.com/illiliti/libudev-zero]'s hotplug out of the box

# License
MIT
