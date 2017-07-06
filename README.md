# README

A set of utilities for managing background processes.

- `daemonize`: start the given process as a daemon using the double-fork method
- `renew`: restart a process whenever it returns (rate limited)
- `semaphore`: increment or decrement a stored counter
- `pipekill`: send a SIGTERM to a child process when a pipe is written to

## Use cases

This is aimed at simple systems with limited layers of functionality.

- Manage persistent background processes, for instance `sshd`
- Allow triggering persistent programs on device insertion, for instance
  `wpa_supplicant` and `udhcpc`
- Manage background scripts or batch jobs

This is not suitable for managing the startup process of complex systems, due
to the simplified design (no dependency management!) - use an alternative such
as `systemd` for that.

## Programs

The main programs used for interacting with the daemon state are listed below.
These programs explicitely stop and start running services, ignoring
dependencies.

* `bh-start` - start a service
* `bh-stop` - stop a service
* `bh-stopall` - stop all services
* `bh-status` - print the status of a service

To describe dependencies, a refcount-style system is implemented using the
programs listed below.
By "requiring" another service in the pre script and "releasing" the service in
the post script, services can be started and stopped as required, without
requiring an explicit dependency tree.

* `bh-require` - require a service to be started (returns once the service starts)
* `bh-release` - release a prior require (returns once the service exits)

