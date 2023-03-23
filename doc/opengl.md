## Rendering on the Raspberry Pi in OpenGL

When displaying graphics, DMDReader uses OpenGL ES 3 and access the display using the "direct rendering manager" (DRM). This . However, there is one important limitation. Only a single 
process can access the DRM. That means that even on a two-screen setup, you can't have two independent accessing the screens at the same time. 
This complicates stuff as the video player that is needed for PubPacks need to be included directly into the main process.

Below is a list of links to different documentation, sample code and more that helped during the development of the RPI graphical interface.

## Additional resources

I have created a [simple list with links to additional resources in the web](links.md). Feel free to send a pull request with additional resources.
