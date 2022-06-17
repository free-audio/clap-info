# clap-info: A simple command line CLAP information tool

This is a CLAP information tool which simply loads a clap and allows you to
print a variety of information about the plugin. It is primarily useful in 
debugging and when starting a CLAP, but also is handy for smoke tests, attaching
debuggers, and so on.

We welcome pull requests and improvements.

## To build

```bash
git clone https://github.com/surge-synthesizer/clap-info
cd clap-info
git submodule update --init --recursive
cmake -Bbuild
cmake --build build
```

## To use

The most basic usage, using all the default flags, is:

```bash
build/clap-info "/path/to/Surge XT.clap"
```

But you can configure the information with various degrees of probes, verbosity, and more. 
In this regard, the code is the documentation, but luckily our argument parser means the
code is also our help screen! Try:

```bash
build/clap-info --help
```

