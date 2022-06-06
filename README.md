# A simple command line clap validator

This is a clap validator which I'm using to just sort of show and bootstrap basics
of how to clap host.

## To build

```bash
git clone (repo)
cd micro-clap-host
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

