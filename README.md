# RAMMO

RAMMO (RAM-Monitor) is a lightweight program written in C++ to monitor and kill the processes that are using too much memory causing system crashes or a slow down (due to the swap).

Start it at the beginning of the session and let it keep care of your computer's memory.

The usage is very simple:
`$ rammo <frequency_monitor> <memory_threshold>`
where `<frequency_monitor>` is the time interval (expressed in seconds) to wait to monitor the current running processes and `<memory_threshold>` is the maximum amount of memory (expressed in MB) that a process can use, otherwise it will be killed.

To test if Rammo is working try to run:

`$ for i in {1..99999999}; do echo -n "$i" | sha1sum; done;`

**Attention:** this command will exhaust your memory, unless rammo (or you) kill the bash.

### License
GPLv3
