# MQ2AutoForage

INI driven autoforaging

## Commands

```txt
/startforage - commence autoforaging.
/stopforage - stop autoforaging.
/keepitem {item} - add/change the item in the .ini file to auto-keep.
/destroyitem {item} - add/change the item in the .ini file to auto-destroy.
```

## Sample INI

```ini
[General]
AutoKeepAll=on
AutoAddAll=on
[The Emerald Jungle]
Pod of Water=keep
[The Feerrott]
Roots=keep
Tuft of Grizzly Bear Fur=destroy
Rabbit Meat=keep
Fishing Grubs=keep
[Plane of Fear]
Roots=keep
Fishing Grubs=destroy
[Commonlands]
Roots=keep
Fishing Grubs=destroy
Tuft of Black Bear Fur=destroy
Black Bear Skull=destroy
[Temple of Veeshan]
Pod of Water=keep
Roots=keep
Wurm Egg=keep
Glob of Slush Water=keep
Lichen Roots=keep
Dragon Claw Sliver=keep
Drake Egg=keep
Dragon Egg=keep
[The Ruins of Old Paineel]
Fishing Grubs=destroy
```