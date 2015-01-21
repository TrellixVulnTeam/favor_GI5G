Favor
==
Copyright (c) 2015  Joshua Tanner (mindful.jt@gmail.com)

What is Favor
==
This is more accurately a paragraph about what favor _will be_, as I'm very much still in the middle of development. The idea is to produce an application that can store information about, and perform analytics on, any type of communication that can be represented as sent and received messages. This repository is a C++ backend which is intended to run as the code behind both an Android application (hence the JNI bindings) and an eventual desktop application. I hope that Favor will distinguish itself for interesting analytics, but the truly unique thing about it will be its breadth of applicable communications platforms: the data is stored in a generic format in Favor's own SQLite database, and as long as we have an adapter (called Managers right now) to pull information from a given communications platform, we can perform analytics on that platform. Right now there's an email adapter that runs on the desktop and a text message adapter (primarily written in Java, as part of the Android application) that runs on Android phones, but my hope is to eventually cover Line, Skype (on desktop), and potentially many more. 

TODO list, in no particular order:
==
This is currently very messy because I'm mostly using it for myself. Also changes frequently.
 - Figure out how we want to handle failures of the AndroidTextManager the same way we do with the C++ based AccountManagers.
 - Look at how Google Test works with the NDK, because it does work with the NDK.
 - Look at better ways to handle recovering from bad databases. For now it would be enough if we could delete the database file and rebuild it without messing up the active DB connections
 (though this may be difficult/not worth it to do threadsafely). Eventually we should look into something like trying each table and recovering whatever data we can save, but that's much
 further down the road.
 - The reader tests need at least one test that looks for an exact message with all its attributes so we can't miss attribute-loading problems (like our previous no-id issue)
 - Japanese support is going to require we handle the "shiftJIS" ("big5" won't hurt either while we're at it, though it's Chinese) encoding, because VMIME is having none of it. 
 Look into detecting this (and any other encodings that tidyhtml handles but vmime doesn't) and using TIDY to convert the text, trying to avoid any extra HTML work. It'd also be better if we knew tidy worked on Android so we could count on it for doing this in other
 situations, but that just takes some testing. __Part of doing this is teaching the email manager to better recognize encodings, and if it hits one it doesn't know, it needs to treat that
 as a failure - right now it just merrily exports it knowing we won't be able to save it properly.__ This'll also be our chance to write EmailManager tests. Yay.
 - In the worker address table computing code, we need to figure out what we're doing with suggested names (how to use/store them, whether to save them or give them to the reader, etc.)
 - Some Android specific optimizations at the C++ layer wouldn't hurt: the reader keeping contacts in a hash by id for faster lookup, and the processor potentially pushing
 cache information up to a separate java layer cache proactively. 
 - Spend some time with a query analyzer and make sure that SQLlite is getting the best use possible out of our indices, and think about what might be worth changing if not.
 - Basic threading tests to make sure datalocks do their job
 - In a perfect world, our methods to update contacts would properly adjust the state of the contacts and their held addresses instead of just marking the list as needing to be
 refreshed. This will take a little bit of work to do elegantly though - additions must create a new contact with an address _and_ ensure no other contacts have that address, and of
 course updates just to address linkages must do something similar. Deletions should be relatively simple. 
 
Presentation Principles
==
 - In cases where there is ambiguity about which data to use - such as when, for a received message, we must choose between using the time it was sent to our user and the time our user actually
 received it - the decision should always to be _to use the data in the same way it is presented to the user, or that the user thinkgs about it_. For example, in cases of email, users are typically
 presented with the date in the email header (as opposed to IMAP's internaldate) so that would be best to use, whereas texts are almost always displayed with the timestamp they were actually received. 
 
 
On Threading
==
 Favor is an application before it is any kind of library, so the threading support here is as minimal as is safe. Favor uses an [RAII](http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization)
 approach to locking: the DataLock class is an "exclusive pointer" that comes with a lock to the data it points to. In short if you have a variable from which to access data, you have a lock on that
 data. DataLocks are safe to copy as they use shared_pointers internally but for obvious reasons they should be _claimed as late as possible, and kept around only as long as absolutely
 necessary_. Claiming a DataLock and then performing a slow database operation or even worse, hitting the network, can rapidly turn into hugely wasteful wait times for another thread.
 
 In terms of information that is important to keep threadsafe:
  - Important information is cached. This information can be retrieved, and also refreshed when changes to the database are made. In cases where we know exactly how a change to the database will
  adjust cached data (such as when we addd a new account), the data can be adjusted directly but otherwise it must be recomputed and rewritten. This may not even require explicit locks though,
  only atomic variables. 
  - Account and contact data has locks around it so it can be read reliable from a reader thread and updated from a writer thread
 

On Portability
==
Favor's c++ core is designed to be the backend for an eventual desktop program, as well asn an Android phone application. This means we want the _majority_ of Favor code to be portable to either
platform. However, realistically, there are some cases in which it serves us better to just rewrite code. Additionally, there are some MessageManagers that are decidedly platform specific. Examples:
 - Email will be available on both platforms, but almost certainly implemented differently (and largely in Java) on Android because of how difficult it has proven to get any kind of IMAP/MIME library
 working with the NDK.
 - Skype will be desktop only, because we rely on reading user's Skype logs which we won't have permission to do on Android (they might also be formatted differently, but this is moot if we can't read
 them at all).
 - Text messages will be Android only, for obvious reasons. Also written largely in Java because Android has particularly convenient APIs for ensuring backwards compatibility which are not
 exposed in C++.
 - Line (and other thrift based clients) will be available on both platforms... and hopefully implemented similarly. This is a later part of the project, so we have only limited knowledge right now,
 but it seems there is some evidence of being able to compile thrift clients with the NDK based on Apache tickets here: https://issues.apache.org/jira/browse/THRIFT-1846. Also seems reasonable on the
 grounds that its main dependency is Boost, which has at least some (unofficial) NDK support.
 
 
On JNI Bindings
==
The basic idea here is to do the absolute minimum at the C++ layer. The operations and computations we do here are faster if we can do them natively, but construction of objects and such
should be left ot Java wherever it makes sense and doesn't mean more layer switches, because failures are so much easier to handle there. The important thing is just ot properly signal
if anything goes wrong at the C++ layer so Java knows to stop whatever it's doing. 

Dependencies
==
Dependencies are fetched via a python script in lib. How exactly some of these work may change over time but in general, running the script should have you mostly up and ready to go. 
 - [SQLite](http://sqlite.org/)
 - [UTF8-CPP] (http://utfcpp.sourceforge.net/)
 - [rapidjson](https://github.com/miloyip/rapidjson)
 - [pugixml](https://github.com/zeux/pugixml)
 - A [very slightly modified tidy-html5](https://github.com/Mindful/tidy-html5). The changes here are just to get it to compile on Android.
 - The desktop implementation of the Email MessageManager (EmailManager) uses a [very slightly modified VMIME](https://github.com/Mindful/vmime/)  built using the command in 
 favor/src/lib/vmime/build_cmd. We'll move to using the official library as soon as it has support for IMAP search. Until then, the only changes in the homebrew version are to expose some 
 internals so we can run our own IMAP searches.
 
