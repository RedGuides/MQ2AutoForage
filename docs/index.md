---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2autoforage.99/"
support_link: "https://www.redguides.com/community/threads/mq2autoforage.29773/"
repository: "https://github.com/RedGuides/MQ2AutoForage"
config: "MQ2Forage.ini, MQ2Forage_server.ini, MQ2Forage_character_server.ini"
authors: "watlol, Bl!ng, eqmule, ChatWithThisName, Sic, Knightly, JerkChicken, GoldenFrog"
tagline: "Configurable foraging"
---

# MQ2AutoForage
<!--desc-start-->
Uses the forage skill and your configured .ini file to keep or destroy foraged items. You must have the Forage skill on an ability menu for the plugin to operate correctly.
<!--desc-end-->

## Commands

<a href="cmd-destroyitem/">
{% 
  include-markdown "projects/mq2autoforage/cmd-destroyitem.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2autoforage/cmd-destroyitem.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2autoforage/cmd-destroyitem.md') }}

<a href="cmd-keepitem/">
{% 
  include-markdown "projects/mq2autoforage/cmd-keepitem.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2autoforage/cmd-keepitem.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2autoforage/cmd-keepitem.md') }}

<a href="cmd-startforage/">
{% 
  include-markdown "projects/mq2autoforage/cmd-startforage.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2autoforage/cmd-startforage.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2autoforage/cmd-startforage.md') }}

<a href="cmd-stopforage/">
{% 
  include-markdown "projects/mq2autoforage/cmd-stopforage.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2autoforage/cmd-stopforage.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2autoforage/cmd-stopforage.md') }}

## Settings

The default INI file is: MQ2Forage.ini

To allow for more customizations you can use server specific or a character specific INI. The plugin will search for your characters INI in the following order:

1. MQ2Forage_CharacterName_Server.ini
2. MQ2Forage_Server.ini
3. MQ2Forage.ini

To use a server or per character INI, these files must be created manually.

Prior to 2021-03-16; The plugin only used Character .ini files.

Sample .ini file,

```ini
[General]
AutoKeepAll=on
AutoAddAll=on
[Global]
Fishing Grubs=destroy
[The Emerald Jungle]
Pod of Water=keep
[The Feerrott]
Roots=keep
Tuft of Grizzly Bear Fur=destroy
Rabbit Meat=keep
[Plane of Fear]
Roots=keep
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
```

Zone names are in [Brackets] e.g. [The Feerrott]. 

If you'd like to apply rules globally, use [Global]. Item settings under [Global] will be overruled by zone-specific settings.
