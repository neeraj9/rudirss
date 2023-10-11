
![Header](./logo.png)

## Overview

RudiRSS is a rudimentary RSS reader implemented in C++ on Windows.

## Features

- Fast and lightweight
- Use Microsoft Edge WebView2 to render web contents
- Support RSS2.0/Atom feeds
- Import feeds from OPML or simple list file
- Implement without 3rd party libraries

## Why

I read RSS feeds every day and have used many RSS readers, however, some are heavy and have too many features that I seldom use, some don't maintain anymore. And I just want a simple RSS reader that could render web content without issues. Therefore I spent some time implementing it on Windows in C++, and by pure Win32 APIs and built-in SDKs without 3rd party dependencies (except for the google-test framework to test core libraries.).

Before I implement RudiRSS, I implemented [libmsxml](https://github.com/lambertlulala/libmsxml) to parse feeds and [libWinHttp](https://github.com/lambertlulala/libWinHttp) to grab feeds from web. They are both used by RudiRSS.

## Demo

![Demo1](https://github.com/lambertlulala/rudirss/assets/76695321/406a1566-bfad-4fd2-bf27-0414870d43a9)
![Demo2](https://github.com/lambertlulala/rudirss/assets/76695321/4e06698f-29a8-4d37-960d-28ef77f1dbb1)

## Usage

### Import feeds
You can import feeds by `File` menu -> `Import from OPML` or `Import from list file`.

The list file have content like this
```
https://www.makeuseof.com//feed
https://www.geeksforgeeks.org//feed
https://news.ycombinator.com/rss
...
```

### Search feeds
The search box locates in top-right, choosing `Source feeds` means to filter out source feeds, while `Feed items` means to filter out feed items by title.

### Configuration

You could create an optional configuration file named `rudirss.ini` under the folder `%APPDATA%\rudirss`.

Some settings could be configured like
```
[Database]
AllowDeleteOutdatedFeedItems=1 ;The program will delete outdated feed items at startup
ReserveDays=7300               ;How many days do we want to keep feed items? 7300 days = 20 years

[Display]
FeedWidth=250                    ;The width to display the feed-source panel
FeedItemTitleColumnWidth=350     ;The width of title column in the feed-item panel
FeedItemUpdatedColumnWidth=200   ;The width of updated column in the feed-item panel
FeedSortMethod=1                 ;0: unsorted; 1: ascending sorting order 2: descending sorting order
```

### Update interval
By default, each feed will update every `30` minutes. By design, each feed have its own updated interval and can be set differently, however, the implementation doesn't provide its configuration so far. An other approach to configure it is by modifying its `updateinterval` value in `Feed` table in `rudirss.db`, where `rudirss.db` is a SQLite database, any SQLite browsing tool could modify it.

## Download
- Executable binaries can be downloaded on [latest release](https://github.com/lambertlulala/rudirss/releases/latest) page.

## How to build
1. `git clone https://github.com/lambertlulala/rudirss.git`
2. `git submodule update --init --recursive`
3. Build with Visual Studio Community 2022

## Future plan
- Implement console version of RudiRSS
- Plugin system that allows users to script or load their custom libraries
- Filter that enables users to decide what feeds to be kept in the database
- Add other configurations(in ini file or UI) to control some internal parameters
- Dark mode
