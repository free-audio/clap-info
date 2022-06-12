# A simple command line CLAP information tool

This is a CLAP information tool which simply loads a clap and allows you to
print a variety of information about the plugin. It is primarily useful in 
debugging and when starting a CLAP, but also is handy for smoke tests, attaching
debuggers, and so on.

We welcome pull requests and improvements.

## To build

```bash
git clone (repo)
cd clap-val
git submodule update --init --recursive
cmake -Bbuild
cmake --build build
```

## To use
```bash
build/clap-val "/path/to/Surge XT.clap"
```

or

```bash
build/clap-val --help
```

