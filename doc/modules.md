# Modules

DMDReader is completely modular. It does not have a pre-defined functionality. Everything is handled by 3 types of modules:

- Sources
- Processors
- Renderers

This documentation might not always be up-to-date. To check what sources, processors and renderers are available in the latest version, have a look at (../util/objectfactory.cpp)

## Sources

Sources are - as the name suggests - sources for DMD data. Your configuration have to define at least one source. If you define multiple sources, 
they will be processed sequentially: all frames from the firs sources will be uses, then it will start with the seconds source and so on with all sources

### dat

Reads frame from a DAT file. This has been used in initial stated of the development, but it's recommended to uses the txt source now.

|Parameter|Description|
|---|---|
|name|Name of the file to read (relative to the directory of the configuration file|

### txt

Reads the frames from a TXT file. This format is also used on VPins for colorisations.  

|Parameter|Description|
|---|---|
|name|Name of the file to read (relative to the directory of the configuration file|
|bitsperpixel|Bits/pixel. Can be 2,3 or 4 - depending on the source (e.g. 2 bit for WPC, 4 bit for SAM and Spike 1)

### png

Reads PNG files, each representing a single frame

|Parameter|Description|
|---|---|
|files|A list of files (you can use wildcards) to use. Ideally all these have the same resolution. It's also recommended to only use files in usual DMD resolutions, e.g. 128x32)||


### spi

Reads frames directly from a pinball's DMD interface. An interface board is needed for this

|Parameter|Description|
|---|---|
|device|The SPI device file to use, e.g. /dev/spidev1.0|
|speed|SPI clock speed|1000000 = 1MHz should be enough|
|notify_gpio|GPIO pin the notify signal is connected to (e.g. 7)|

## null

A source that delivers no frames at all. Only useful for debugging purposes. 

## Processors

## Renderers
