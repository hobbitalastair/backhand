# README

A set of utilities for managing background processes.

- `escort`: provide a control socket for a child program
- `connect`: connect to a socket and wait for a response
- `semaphore`: increment or decrement a stored counter
- `state`: provide atomic access to the contents of a file

## Use cases

This is aimed at simple systems with limited layers of functionality.

- Manage persistent background processes, for instance `sshd` and `getty`
- Allow starting/stopping daemons on device insertion, for instance
  `wpa_supplicant` and `udhcpc` for a wifi dongle
- Manage background scripts or batch jobs

This is not suitable for managing the startup process of complex systems, due
to the simplified design (no dependency management!) - use an alternative such
as `systemd` for that.

Key concerns are (in order of importance):

- Complexity
- Reliability and correctness
- Memory usage (process count, structures)
- Speed (mostly bootup/shutdown speed)

## Programs

The main programs used for interacting with the daemon state are listed below.
These programs explicitely stop and start running services.

* `bh-start` - start a service
* `bh-stop` - stop a service
* `bh-stopall` - stop all services
* `bh-status` - print the status of a service

To describe dependencies, a refcount-style system is implemented using the
programs listed below.
By "requiring" another service in the pre script and "releasing" the service in
the post script, services can be started and stopped as required, without
requiring an explicit dependency tree.
These return once the operation has completed.

* `bh-require` - require a service to be started
* `bh-release` - release a prior requirement

## Service scripts

Service scripts are stored in a per-service directory.
These are expected to be executable files that do not fork and return once
completed.

* `pre` - run before a service starts
* `run` - the actual service to run (forking services not supported)
* `post` - run after the service stops

