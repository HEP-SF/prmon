# Process Monitor (prmon)

[![Build Status][build-img]][build-link]  [![License][license-img]][license-url] [![Codefactor][codefactor-img]][codefactor-url]

[build-img]: https://github.com/HSF/prmon/workflows/CI/badge.svg?branch=main
[build-link]: https://github.com/HSF/prmon/actions?query=workflow%3ACI+branch%3Amain
[license-img]: https://img.shields.io/github/license/hsf/prmon.svg
[license-url]: https://github.com/hsf/prmon/blob/main/LICENSE
[codefactor-img]: https://www.codefactor.io/repository/github/HSF/prmon/badge
[codefactor-url]: https://www.codefactor.io/repository/github/HSF/prmon

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.2554202.svg)](https://doi.org/10.5281/zenodo.2554202)

The PRocess MONitor is a small stand alone program that can monitor
the resource consumption of a process and its children. This is
useful in the context of the WLCG/HSF working group to evaluate
the costs and performance of HEP workflows in WLCG. In a previous
incarnation (MemoryMonitor) it has been used by ATLAS for sometime to
gather data on resource consumption by production jobs. One of its
most useful features is to use smaps to correctly calculate the
*Proportional Set Size* in the group of processes monitored, which
is a much better indication of the true memory consumption of
a group of processes where children share many pages.

prmon currently runs on Linux machines as it requires access to the
`/proc` interface to process statistics.

## Build and Deployment

### Cloning the project

As prmon has dependencies on submodules, clone the project as
    
    git clone --recurse-submodules https://github.com/HSF/prmon.git
    
### Building the project

Building prmon requires a C++ compiler that fully supports C++11,
CMake version 3.3 or
higher and the [Niels Lohmann JSON libraries](https://github.com/nlohmann/json).

Building is usually as simple as

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=<installdir> <path to sources>
    make -j<number of cores on your machine>
    make install

If your installation of JSON is in a non-standard location then
setting `nlohmann_json_DIR` may be required as a hint to CMake.
(e.g. this is necessary on Ubuntu18 when using the standard `nlohmann-json-dev`
package: `-Dnlohmann_json_DIR=/usr/lib/cmake`.)

The option `-DCMAKE_BUILD_TYPE` can switch between all of the standard
build types. The default is `Release`; use `RelWithDebInfo` if you want
debug symbols.

To build a statically linked version of `prmon` set the `BUILD_STATIC`
CMake variable to `ON` (e.g., adding `-DBUILD_STATIC=ON` to the
command line).

Note that in a build environment with CVMFS available the C++ compiler
and CMake can be taken by setting up a recent LCG release.

The tests in prmon now run in Python3. If your environment does not
support Python3 properly it is possible to use Python2 for the tests
by setting `-DPYTHON_TEST=python2`.

### Creating a package with CPack

A cpack based package can be created by invoking

    make package

### Running the tests

To run the tests of the project, first build it and then invoke

    make test

As noted above these will use `python3` by default, but this can
be changed via the `PYTHON_TEST` option to CMake.

## Running

The `prmon` binary is invoked with the following arguments:

```sh
prmon [--pid PPP] [--filename prmon.txt] [--json-summary prmon.json] \
      [--interval 30] [--suppress-hw-info] [--units] [--netdev DEV] \
      [--disable MON1] [--level LEV] [--level MON:LEV]\
      [-- prog arg arg ...]
```

* `--pid` the 'mother' PID to monitor (all children in the same process tree are monitored as well)
* `--filename` output file for timestamped monitored values
* `--json-summmary` output file for summary data written in JSON format
* `--interval` time, in seconds, between monitoring snapshots
* `--suppress-hw-info` flag that turns-off hardware information collection
* `--units` add information on units for each metric to JSON file
* `--netdev` restricts network statistics to one (or more) network devices
* `--disable` is used to disable specific monitors (and can be specified multiple times); 
    the default is that `prmon` monitors everything that it can
  * Note that the `wallmon` monitor is the only monitor that cannot be disabled
* `--level` is used to set the logging level for monitors
  * `--level LEV` sets the level for all monitors to LEV
  * `--level MON:LEV` sets the level for monitor MON to LEV
  * The valid levels are `trace`, `debug`, `info`, `warn`, `error`, `critical`
* `--` after this argument the following arguments are treated as a program to invoke
  and remaining arguments are passed to it; `prmon` will then monitor this process
  instead of being given a PID via `--pid`

`prmon` will exit with `1` if there is a problem with inconsistent or 
incomplete arguments. If `prmon` starts a program itself (using `--`) then
`prmon` will exit with the exit code of the child process.

When invoked with `-h` or `--help` usage information is printed, as well as a
list of all available monitoring components.

### Environment Variables

The `PRMON_DISABLE_MONITOR` environment variable can be used to specifiy a comma
separated list of monitor names that will be disabled. This is useful when
`prmon` is being invoked by some other part of a job or workflow, so the user
does not have direct access to the command line options used. e.g.

```sh
export PRMON_DISABLE_MONITOR=nvidiamon
other_code_that_invokes_prmon
...
```

Disables the `nvidiamon` monitor.
## Outputs

In the `filename` output file, plain text with statistics written every
`interval` seconds are written. The first line gives the column names.

In the `json-summary` file values for the maximum and average statistics
are given in JSON format. This file is rewritten every `interval` seconds
with the current summary values. Use the `--units` option to see exactly
which units are used for each metric (the value of `1` for a unit means
it is a pure number).

Monitoring of CPU, I/O and memory is reliably accurate, at least to within
the sampling time. Monitoring of network I/O is **not reliable** unless the
monitored process is isolated from other processes performing network I/O
(it gives an upper bound on the network activity, but the monitoring is
per network device as Linux does not give per-process network data by
default).

### Visualisation

The `prmon_plot.py` script (Python3) can be used to plot the outputs of prmon from the
timestamped output file (usually `prmon.txt`). Some examples include:

* Memory usage as a function of wall-time:
```sh
prmon_plot.py --input prmon.txt --xvar wtime --yvar vmem,pss,rss,swap --yunit GB
```
![](example-plots/PrMon_wtime_vs_vmem_pss_rss_swap.png)

* Rate of change in memory usage as a function of wall-time:
```sh
prmon_plot.py --input prmon.txt --xvar wtime --yvar vmem,pss,rss,swap --diff --yunit MB
```
![](example-plots/PrMon_wtime_vs_diff_vmem_pss_rss_swap.png)

* Rate of change in CPU usage as a function of wall-time with stacked
user and system utilizations:
```sh
prmon_plot.py --input prmon.txt --xvar wtime --yvar utime,stime --yunit SEC --diff --stacked
```
![](example-plots/PrMon_wtime_vs_diff_utime_stime.png)

The plots above, as well as the input `prmon.txt` file that is used
to produce them, can be found under the `example-plots` folder.

The script allows the user to specify variables, their units, plotting
style (stacked vs overlaid), as well as the format of the output image.
Use `-h` for more information.


### Data Compression

The `prmon_compress_output.py` script (Python3) can be used to compress the output file
while keeping the most relevant information.

The compression algorithm works as follows:
* For the number of processes, threads, and GPUs, only the measurements that are different with respect to the previous ones are kept.
* For all other metrics, only the measurements that satisfy an interpolation condition are kept.

This latter condition can be summarized as:
* For any three neighboring (and time-ordered) measurements, A, B, and C, B is deleted if the linear interpolation between A and C is consistent with B ± *threshold*. Otherwise, it's retained. The *threshold* can be configured via the `--precision` parameter (default: 0.05, i.e. 5%)


The time index of the final output will be the union of the algorithm outputs of the single 
time series. Each series will have NA values where a point was deleted at a kept index and, unless otherwise
specified by the `--skip-interpolate` parameter, will be linearly interpolated to maintain a consistent number of data points
and the result will be rounded to the nearest integer for consistency with the original input.

If the `--skip-interpolate` parameter is passed, deleted values will be written as empty strings in the output file, and will be interpreted
as `NA` values when imported into Pandas.

Example:
```sh
prmon_compress_output.py --input prmon.txt --precision 0.3 --skip-interpolate
```


## Feedback and Contributions

We're very happy to get feedback on prmon as well as suggestions for future
development. Please have a look at our [Contribution
Guide](doc/CONTRIBUTING.md).


### Profiling

To build prmon with profiling set one of the CMake variables
`PROFILE_GPROF` or `PROFILE_GPERFTOOLS` to `ON`. This enables
GNU prof profiling or gperftools profiling, respectively.
If your gperftools are in a non-standard place, pass a hint
to CMake using `Gperftools_ROOT_DIR`.


# Copyright

Copyright (c) 2018-2020 CERN.
