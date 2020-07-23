# Small N Stats Session Recorder

Session Recorder is a Qt-based application made to assist researchers in video recording applications. Using basic web-cams (often free), sessions can be recorded dynamically an annotated with various details.

Features include:
  - Timestamps for all recording of sessions
  - Tagging of Session Numbers, Treatments, Treatment conditions, and De-identified participant numbers
  - Sorts participant videos by ID numbers and treatments, for ease of use and reference

### Version
------
0.0.1 (alpha)

### Changelog
------
 * 0.0.1 - Initial Push

### Referenced Works (F/OSS software)
------
The Small N Stats Discounting Model Selector uses a number of open source projects to work properly:
* Meeting-Recorder - MIT Licensed. Copyright 2015-2016 University of Helsinki. [GitHub](https://github.com/HIIT/meeting-recorder)
* OpenCV - BSD-3 Clause. Copyright Open Source Computer Vision. [GitHub](https://github.com/opencv/opencv)
* Qt Framework (5.8) - LGPL 3.0 (GPL 3.0 Licensed). Copyright (C). Qt Team [Site](https://www.qt.io/)
* Qt Examples (AvRecorder) - BSD-3 Clause. Copyright (C). Qt Team [Site](https://www.qt.io/)

### Acknowledgements and Credits
------
* Helsinki Institute for Information Technology - This work is largely a port of their Meeting-Recorder software.

### Installation
------
No other packages are required. Simply build and run or install and run. However, users must have aquire FFmpeg and have this binary on their PATH to allow the program to mux audio/visual streams together.

### Setup, Mac OSX
------
You will can install FFmpeg using Homebrew. This is recommended, as the program will look to /usr/local/bin for FFmpeg. If you do not have Homebrew installed, you can install via the following:

ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you already have Homebrew installed, you can install FFmpeg using the following:

brew install ffmpeg

### Setup, Windows
------
TODO

### Download
------
All downloadable binaries, if/when posted, will be hosted at [Small N Stats](http://www.smallnstats.com).

### Development
------
Want to contribute? Great! Emails or PM's are welcome.

### Todos
------
 - ~~Resolution options~~
 - ~~Windows-specific testing/recording options~~
 - Warn on over-write
 - Memory for past recording (position, numbering, etc.)
 - Bug testing

### License
----
Session Recorder - Copyright 2018, Shawn P. Gilroy. GPL-Version 3
