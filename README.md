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

Prebuilt Releases
-----------------

You can find prebuilt releases available for download here:

https://github.com/CyberBoardPBEM/cbwindows/releases

Fetching the Code
-----------------

For portability reasons, CyberBoard is transitioning to use wxWidgets instead
of MFC.  The source code of wxWidgets is accessed using the `git submodule`
mechanism.  To properly fetch the code, use
`git clone https://github.com/CyberBoardPBEM/cbwindows.git --recurse-submodules`

Building the Code
-----------------

The codebase has been updated to work with Microsoft's Visual Studio 2022 Community
Edition. That way anyone with a Windows system and some programming chops can
build the software using the "free" Microsoft tools. I say "free" because Microsoft requires
users of the Community Edition to create an account with them to activate the software
for use after 30 days. If you can live with that you should be good to go.

You can get Visual Studio 2022 Community Edition here:

https://visualstudio.microsoft.com/vs/community/

Due to the recent dependency on wxWidgets, building CyberBoard now requires
CMake.  Therefore, when installing Visual studio, include the MSVS `C++ CMake
tools for Windows` component.

**To build the debug configuration at the command line:**
1. Open the command line `x64 Native Tools Command Prompt for VS 2022`.  This
should be installed in the Windows Start menu, in the `Visual Studio 2022`
folder
2. `cd <root-of-cyberboard-checkout-directory>`
3. `cmake -G Ninja -B out/build/x64-Debug`
4. `cmake --build out/build/x64-Debug`

The output will be in `<working copy>\out\build\x64-Debug\runtime`

**To build the release configuration at the command line:**
1. Open the command line `x64 Native Tools Command Prompt for VS 2022`.  This
should be installed in the Windows Start menu, in the `Visual Studio 2022`
folder
2. `cd <root-of-cyberboard-checkout-directory>`
3. `cmake -G Ninja -B out/build/x64-Release -DCMAKE_BUILD_TYPE=RelWithDebInfo`
4. `cmake --build out/build/x64-Release`

The output will be in `<working copy>\out\build\x64-Release\runtime`

**To build the code in the Visual Studio IDE:**
1. Run Visual Studio
2. Use menu `File` | `Open` | `Folder...` and choose `<working copy>`.
3. Visual Studio will run CMake's Ninja generation process automatically, but
it may take a considerable time.  (The
`Output Window` | `Show output from:` | `CMake` choice will display the CMake
output.)  Wait for the `Build` menu to have a `Build All` item.
4. Use menu `Build` | `Build All`

The output will be in `<working copy>\out\build\x64-Debug\runtime`

To debug the code in the Visual Studio IDE:
1. Build the code in the IDE
2. Use menu `View` | `CMake Targets`
3. In the `Solution Explorer - CMake Targets View` panel, right click on
`cbwindows` | `CyberBoard Project` | `CBDesign (executable)`, and select `Set as Startup Item`
4. Open `GM\Gm.cpp`
5. Set a breakpoint in `CGmApp::InitInstance()`
6. Use menu `Debug` | `Start Debugging`

Alternatively, *after performing one of the builds above*, the `CyberBoard.sln`
may be used as usual.  (The initial build generates the necessary
wxWidgets files on which `CyberBoard.sln` depends.)  Set the Configuration to
`DebugU` and Platform to `x64`, and the output will be in
`<working copy>\x64\DebugU`.


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
