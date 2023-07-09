# VNI/PAL support

There is some rudimentary VNI/PAL colorisation support. There is no guarantee it will work with a specific colorisation. The 
format was never fully documented. Therefore, lots of the code is based on [Freezy's DMD Extensions](https://github.com/freezy/dmd-extensions).
Feel free to improve it if you like.

## Getting VNI/PAL files

Many authors do not upload VNI/PAL files to vpuniverse. I can only speculate why this is the case. The newer PAC format is an 
undocumented, encrypted format to enable DRM functionalities. It won't work with the DMDReader software.

In general there are 2 legal ways to get PAL/VNI files
1. You did download these in the past and still have these laying around - just use them
2. Contact the author and ask them. Some might be willing to just share these files. 
 Others might want to get paid for it.  And others might not be willing to share at all. 

## Timing

By default, duplicated frames are suppressed internally. This already happens on the hardware interface. It only sends modified frames to
the Raspberry Pi. Some VNI colorisations won't work correctly if frames are not delievered at a fixed rate. You can enable this behavior in the "general" 
section of the configuration:

```
"general":{
  "skip_unmodified_frames": false,
  "frame_every_microseconds": 16000
},
```

This example configured the software that it will deliver frames every 16ms.
