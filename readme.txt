http://foosion.foobar2000.org/0.9/tutorials

REQUIRES SHARED.DLL

To build:
just build it. the output DLL gets placed in ./Foobar2000/components
so you can just set your debug command to run foobar2000 directly and debug it.

For WMP, regsvr32 the thing (which is done automatically during building)
and it will be available in plugin settings.


TODO:
- tray icon
- allow log window to be shown / hidden
- some help on formatting and filtering
- grumble support