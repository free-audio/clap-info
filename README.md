# BaconPauls test microhost

This is a clap microhost which I'm using to just sort of show and bootstrap basics
of how to clap host.

Here's instructions

1. Install Surge XT and the clap. (Install other claps if you want but one of the modulations in).
main is tied to a particular surge paramid).
2. Build this
```bash
git clone (repo)
cd micro-clap-host
git submodule update --init --recursive
cmake -Bbuild
cmake --build build
```
3. Then run it
```bash
build/micro-clap-host "/path/to/Surge XT.clap"
```

there's a good chance it will then play some random notes out your speakers and make some stdout
