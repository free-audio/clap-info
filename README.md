# clap-info: A simple command line CLAP information tool

This is a CLAP information tool which simply loads a clap and allows you to
print a variety of information about the plugin. It prints the information in 
machine-readable (JSON) format.

`clap-info` can perform the following

- Traverse your system finding .clap files, and scanning them for information
- Load a single CLAP and display the descriptors
- For a given CLAP and descriptor, create an instance of a plugin and print
  extension information, including ports, parameters, and more

We occasionally tag releases with versions and make binary releases available,
but the program is designed to be easy to build, and when starting a CLAP development
effort, can be a useful tiny host to debug the first stages of CLAP development.

## To build

```bash
git clone --recurse-submodules https://github.com/surge-synthesizer/clap-info
cd clap-info
cmake -Bbuild
cmake --build build
```

## To use as a scanner

`clap-info -l` will list all installed CLAPs on your system, using the CLAP 
locations documented in `entry.h`

`clap-info -s` will load each clap and print the descriptors for each CLAP,
and as such implements a CLAP scanner.

## To use on a single CLAP

```bash
clap-info "/path/to/YourCLAP.clap"
```

this usage will print complete information on plugin 0 in your clap.

You can configure the information with various degrees of probes, verbosity, and more. 
Use `clap-info --help` to get full information on the options.

