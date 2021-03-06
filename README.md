Simple multi-threaded web proxy
---

We've implemented a modular, multi-threaded basic http proxy. It accepts proxied
http GET requests (of the form `GET http://hostname/ HTTP/1.0`) in a concurrent
manner, adding incoming connection requests to a job queue. Everything's logged
according to the spec, however, the logging subsytem has been designed to
enable the recording of more log information in a threadsafe and standard manner.

A pool of worker threads, instantiated at run time, are alerted via pthread messages to new work
added to this queue. A single worker at a time is allowed to remove and process
jobs from this queue (producer-consumer style). The pool of threads approach
is used to minimise overheads associated with a process spawning implementation
(fork()'ing) or "fresh thread for each task" implementation.
(We've actually worked on this thread pool together up on https://github.com/lberezy/thread_pool
during the project, and plan to commit back some further changes to it based on
things we've fixed during the project - pull requests are fun!)

We've run some rudimentary tests by setting our proxy as a HTTP proxy for OS X
to use and browsed websites with it. It seems to handle the load of many concurrent
connections quite well, and it's quite funny sometimes to see a link redirect us
to the hostname's root page (the proxy only fetches the / root directory).

---

File structure
##The proxy file for testing is build into "bin".

|____bin
|____src
|____tests
| |____supplied_bins
