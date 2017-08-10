# Notes

# Service controller design and implementation notes

There were three options here:
- Global service controller (manages multiple child processes)
- Event/state machine with pluggable event source/sinks 
  (can manage processes as a side effect)
- Per-service managers with a framework for dependency resolution

Restarting processes and stopping processes cannot be separated except in the
event/state machine design (even then the functionality is mixed into the
event/state machine).

Confusing persistent and temporary processes is a mistake; these have too many
differences to be treated the same.
This in particular is part of the restarting/stopping issue, since otherwise it
is fairly trivial to differentiate between them.

Not having a persistent middleman introduces some issues, eg:
  * How do we tell (in a non-racy manner) what the state of a service is?
  * How to stop all services without risking new services being started?
There are solutions but they involve lock files and aren't very clean...

The event/state machine design seems nice but really just moves the complexity
inwards a step, and could blow the complexity out of proportion.

Determining start order is trivial, however there is a difference between
requiring (dependency relationship) and starting (explicit action) services;
one is human-friendly, the other is for determining startup order.

Process groups seem like the best way to generalize managing single services,
but have subtle issues around figuring out when a service actually exits.

In the global service controller model there is less conceptual complexity as
the various process states are all stored in the controller process, however
getting proper multiplexed communication/subscription working is difficult,
especially when mixed with launching child processes in a clean environment
(as fds get passed to the child).
In this case the global service controller can also just fork/setup/exec each
child process locally, instead of running a supervisor for each process.

The idea that "`malloc`ing is dangerous" seems wrong - when we start a process
we also allocate a bunch of memory! Just try to avoid operations that fail in
general instead...

