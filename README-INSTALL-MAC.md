# Mac install instructions

This should be in the wiki. After that happens, this file can be deleted.

## Mac install (assuming you have brew)

Nearly everything you need can be found via `brew` package handler.
The following should be installed to allow the compilation and running of gsc.

```
brew install libconfig libev xmlrpc-c hamlib libusb dfu-util cmake
```

The following is useful independent of gsc; however, will allow easy REST API testing

```
brew install jq
```

Homebrew's preferred install location is `/usr/local` for macOS Intel and `/opt/homebrew` for newer Apple Silicon macOS.
This is handled within the `CMakeLists.txt` file.

## Packages you need to build yourself - libpredict & json-c

Install these projects by following their install instructions (i.e. their README files)

* https://github.com/la1k/libpredict
* https://github.com/json-c/json-c

Both can be installed in their default locations.
Both have `sudo` style install commands (see README's).

## Final notes

While the Mac is a perfectly acceptable platform to operate gsc, an end user should be aware of the specifics of running hamlib and the gnuradio tools on a Mac.
That is beyond the scope of this file.

