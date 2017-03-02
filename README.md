# README

A set of utilities for managing background processes.

- `daemonize`: start the given process as a daemon using the double-fork method
- `renew`: restart a process whenever it returns (rate limited)
- `semaphore`: increment or decrement a stored counter

## Use cases

This is aimed at simple systems with limited layers of functionality.

- Manage persistent background processes, for instance `sshd`
- Allow triggering persistent programs on device insertion, for instance
  `wpa_supplicant` and `udhcpc`
- Manage background scripts or batch jobs

This is not suitable for managing the startup process of complex systems, due
to the simplified design (no dependency management!) - use an alternative such
as `systemd` for that.

