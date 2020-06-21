CyberBoard Play by EMail System
===============================

Prior to this open source release of CyberBoard the software has been closed source
code that depended on a commercial set of Microsoft Foundation Classes (MFC) extensions 
called the Xtreme Toolkit that was provided by Code Jock. Since the program was always 
provided to users without cost this approach became a burden on future development 
of the software since it cost money to keep everything up-to-date.

As a consequence the CyberBoard codebase hasn't been updated in nearly two decades. 
Despite that, I'm pleased to see the CyberBoard was future proofed enough that 
the program endured that time span and is still in wide use today.

Now recently retired, I undertook the work of removing use of the CodeJock toolkit 
and switching the UI elements to use the MFC Feature Pack from the Visual Studio 
2008 release. The MFC Feature Pack provided many of the essential features that 
Xtreme Toolkit provided.

This repository has the results of that work. Note that the history of the repository
has been truncated to start at the point where the code cleanup is largely completed
and the new MIT license for the source code has been put in force.

In the future I may share the entire history in a read-only fashion just for the
capturing the program's history for history's sake. Currently I don't want to wrestle 
with that problem.

Building the Code
-----------------

The codebase has been updated to work with Microsoft's Visual Studio 2019 Community
Edition. That way anyone with a Windows system and some programming chops can
build the software using the "free" Microsoft tools. I say "free" because Microsoft requires
users of the Community Edition to create an account with them to activate the software
for use after 30 days. If you can live with that you should be good to go.

You can get Visual Studio 2019 Community Edition here: 

https://visualstudio.microsoft.com/vs/community/

Coding Conventions
------------------

The code follows the same coding conventions that the MFC code uses. However, no TAB
characters are allowed in the code. All indentations must be done with spaces set 
to 4 spaces per tab stop. That's the extent of the coding rules.

Goals
-----

The first goal is to get a stable version of CyberBoard going that has the
same feature set as the existing historical release of version 3.10. That means
it needs to be exercised against a variety of gameboxes and games. That's the first
place the community can help.

The first open source release version of CyberBoard will start its release numbering 
at version 3.50.

At that point newer features can be added to evolve the system's capabilities.

In essence first focus on stability, then extend the feature set.

My ultimate hope is that a group of enthusiasts will adopt the program and its
future as I'm getting "long in the tooth" and the fewer loose ends the better. ;)

The CyberBoard Web Site
-----------------------

The CyberBoard web site has been relocated to GitHub and can now be found 
through my `norsesoft.com` domain here:

http://cyberboard.norsesoft.com/

Dale Larson
6/21/2020
