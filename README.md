# ExplorerFrame
**Experimental project. Not for production use.**

ExplorerFrame is a program that resolves some improper behaviors caused by File Explorer regarding extending DWM frames into its client area.

![A File Explorer window with DWM frames extended into its client area using ExplorerFrame](https://user-images.githubusercontent.com/29563098/201342798-7d3deaa2-8f09-4167-accd-0f849022d609.png)

## Limitations
- ExplorerFrame only works in Dark mode. Light mode will cause the text to render improperly.
- ExplorerFrame doesn't resolve the gray backgrounds of some UI elements (like scrollbars).

## How does it work?
It works by hooking into Explorer and detouring Windows' `DwmExtendFrameIntoClientArea` with a custom implementation that ignores calls from File Explorer.

## Are the changes reversible?
Yes. The changes only persist in memory, meaning that if you close ExplorerFrame and restarts Explorer, all changes are undone.

## Licensing
ExplorerFrame is released under the terms of the [GNU General Public License, version 3](LICENSE.txt). You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

## Acknowledgements
[TranslucentTB](https://github.com/TranslucentTB/TranslucentTB) has served as inspiration for me to start work on this project. Many parts of TranslucentTB has also been adopted for this program. Special thanks to TranslucentTB's maintainers and contributors!

Microsoft's [Detours](https://github.com/microsoft/Detours) is used for `DwmExtendFrameIntoClientArea` detouring. Go check it out.

And last but not least, the users of [Mica For Everyone](https://github.com/MicaForEveryone/MicaForEveryone)! It really is astounding to see how much traction it has gained over the years.
