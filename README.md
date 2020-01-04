Advanced Development Mod
======================

ADM is a Half-Life mod base that aims to bring quality-of-life features while maintaining compatibility with the Half-Life singleplayer campaign. It is what powers the mod Utopia at Stake. ([ModDB page](https://www.moddb.com/mods/admerthegame)) 

Current version: v0.03 November. 

## **Features:**

- High-quality sound with FMOD, without fmod.dll being in the HL root directory*
- Physics with Bullet physics engine*
- Time-based entities
- Flexible vehicle system, with support for many vehicle types and configurations
- Parametric view bobbing, swaying and HUD colours

\*Only partially implemented. FMOD for music, physics only work with specifically sized cubes and map geometry is yet to be loaded for collisions.

## **Tools:**

- Bobulator - a GUI tool for making view bob configurations, with a realtime preview (in development)
- EELman - the EEL editor (EEL stands for Extended Entity Lists) (planned)
- VHLT-ADM - fork of VHLT v34 that allows for entity boundaries more than 8192 units, and more in the future


## **Plans:**

- __v0.1__
  - Vehicle system: seats, cars, boats and a helicopter, all destroyable
  - Sound system: music playback, a couple of audio entities, and a soundcache implementation
  - Physics: brush-based physicalised objects, with map collision and player collision
  - Entities: PostActivate() and some other additional entity methods

- __v0.2__
  - Vehicle system: airplanes, space ships, submarines, bikes
  - Sound system: sound-emitting functions on the clientside to call FMOD instead of MSS
  - SDK: attempt to documentate certain things, and clean up some other things
  - Utilities: additions to the Vector class, and classes for quaternions and matrices

- __v0.3__
  - AI: extend abilities of certain NPCs, new AI base class
  - Computer system: basic terminal with a small set of commands
  - Entities: Extended Entity Lists, which will allow for entity I/O, clientside entity spawning, and more
  - Sound system: most serverside sound emits to call FMOD instead of MSS
  - Rendering: gait sequences for monsters

Half Life 1 SDK LICENSE
======================

Half Life 1 SDK Copyright© Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE CORPORATION (“Valve”).  PLEASE READ IT BEFORE DOWNLOADING OR USING THE HALF LIFE 1 SDK (“SDK”). BY DOWNLOADING AND/OR USING THE SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.

You may, free of charge, download and use the SDK to develop a modified Valve game running on the Half-Life engine.  You may distribute your modified Valve game in source and object code form, but only for free. Terms of use for Valve games are found in the Steam Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

You may copy, modify, and distribute the SDK and any modifications you make to the SDK in source and object code form, but only for free.  Any distribution of this SDK must include this license.txt and third_party_licenses.txt.  
 
Any distribution of the SDK or a substantial portion of the SDK must include the above copyright notice and the following: 

DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED “AS IS”.  VALVE AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
 
If you would like to use the SDK for a commercial purpose, please contact Valve at sourceengine@valvesoftware.com.


Half-Life 1
======================

This is the README for the Half-Life 1 engine and its associated games.

Please use this repository to report bugs and feature requests for Half-Life 1 related products.

Reporting Issues
----------------

If you encounter an issue while using Half-Life 1 games, first search the [issue list](https://github.com/ValveSoftware/halflife/issues) to see if it has already been reported. Include closed issues in your search.

If it has not been reported, create a new issue with at least the following information:

- a short, descriptive title;
- a detailed description of the issue, including any output from the command line;
- steps for reproducing the issue;
- your system information.\*; and
- the `version` output from the in‐game console.

Please place logs either in a code block (press `M` in your browser for a GFM cheat sheet) or a [gist](https://gist.github.com).

\* The preferred and easiest way to get this information is from Steam's Hardware Information viewer from the menu (`Help -> System Information`). Once your information appears: right-click within the dialog, choose `Select All`, right-click again, and then choose `Copy`. Paste this information into your report, preferably in a code block.

Conduct
-------


There are basic rules of conduct that should be followed at all times by everyone participating in the discussions.  While this is generally a relaxed environment, please remember the following:

- Do not insult, harass, or demean anyone.
- Do not intentionally multi-post an issue.
- Do not use ALL CAPS when creating an issue report.
- Do not repeatedly update an open issue remarking that the issue persists.

Remember: Just because the issue you reported was reported here does not mean that it is an issue with Half-Life.  As well, should your issue not be resolved immediately, it does not mean that a resolution is not being researched or tested.  Patience is always appreciated.