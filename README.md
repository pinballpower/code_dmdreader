# DMDReader

DMDReader is a software that reads the pinball DMD data provided in different formats including inputs from a real DMD (using https://github.com/pinballpower/code_dmd)

## License
The license has been changed from MIT to GPL v3 on 2.5.2022. In general, I'm not a big fan of the GPL and rather prefer the more open MIT license. 
However, in similar products open source software has been used to create close-source software that should replace existing open-source software. 
To make sure that won't happen with this software, the license of this software has been changed to GPL v3. Just to clarify: That doesn't mean 
anything about "commercial" or "non-commercial" use cases. If you want to create a commercial product based on this software, feel free to do so. 
If you want to sell compiled versions of this software, you can do this as long as you provide users a way to access the full source code that's 
needed to build their own software. This is also valid for any derivate work.

There is an alternative version of this software that includes code that we can't open source. This version isn't licensed under GPL or any other open source license.  It's the code that's used in our binaries of https://github.com/pinballpower/ppos 

We're happy to accept contributions via Pull requests to this project. However, due to the dual-licensing, all code that will be merged into our project
has to be compatible with this dual licensing. That means you additional code needs to use only licenses that also allow the use in closed source software.
Some examples are:
- BSD/MIT
- LGPL

I understand that this isn't a perfect solution. If you don't agree with this policy, you can simply fork the GPL part of this project and work on your own 
version. That's just fine. We simply can't merge pull requests into this repository that do not qualify for closed-source use cases.
I hope we can fully open-source this software at some point, but that's not something we can decide by ourself.

## Rendering on the Raspberry Pi

When displaying graphics, DMDReader uses OpenGL ES 3 and access the display using the "direct rendering manager" (DRM). This . However, there is one important limitation. Only a single 
process can access the DRM. That means that even on a two-screen setup, you can't have two independent accessing the screens at the same time. 
This complicates stuff as the video player that is needed for PubPacks need to be included directly into the main process.

Below is a list of links to different documentation, sample code and more that helped during the development of the RPI graphical interface.

## Additional resources

I have created a [simple list with links to additional resources in the web](https://github.com/pinballpower/code_dmdreader/blob/main/links.md). Feel free to send a pull request with additional resources.
