http://foosion.foobar2000.org/0.9/tutorials

REQUIRES SHARED.DLL

To build:
just build it. the output DLL gets placed in ./Foobar2000/components
so you can just set your debug command to run foobar2000 directly and debug it.

For WMP, regsvr32 the thing (which is done automatically during building)
and it will be available in plugin settings.


TODO:
- "send command now" option with hotkey
- when setting prefs, reset the timer in a logical way. display info too. maybe link to showing the log file?
- use foobar's actual string formatting shit instead of my own
- some help on formatting and filtering
- grumble support

- tray icon
- allow log window to be shown / hidden


substitution format:
$(vtype;format;attribute)

vtype is like "sec" if the attribute is seconds

format is how to format it. it is like a printf format field in that it's your own string plus some substitution
it's the most complex. the substitution here is always %ab - always 2 characters. backslash is the escape character.
      %ab where a=fieldname, b=format
      a:
       depends on the vtype. but for durations, this lets you extract elements out of a time attribute. um i think
       track duration is really the only thing this is good for. See ProcessDurationField() for a list of possible values.
      b:
        x=default
        t=textual representation
        n=n-digit
        1=1-digit, 0-prefix
        ...
        9=9-digit, 0-prefix

      basically, to display the track duration (which is given in seconds), in the format "3.35", use:
      $(s;%Nn:%s2;duration)
      where s specifies that the attribute provided is in seconds.
            %Nn outputs the minutes ("N"), as a normal number ("n")
            : outputs the character ":"
            %s2 outputs the seconds of the minute ("s") as a fixed 2-digit width number
            duration is the name of the metadata attribute to use


attribute is the name of the metadata element