Just getting things set up right now. 

Todo (in order):
 - Japanese support is going to require we handle the "shiftJIS" ("big5" won't hurt either while we're at it, though it's Chinese) encoding, because VMIME is having none of it. 
 Look into detecting this (and any other encodings that tidyhtml handles but vmime doesn't) and using TIDY to convert the text, trying to avoid any extra HTML work. It'd also be better if we knew tidy worked on Android so we could count on it for doing this in other
 situations, but that just takes some testing.
 - Serious email testing with more logging, using all the addresses we have now (I.E. make the fetch method use all of them instead of just contacts')
 - Basic unit tests
 - What do we when we can't parse a message for whatever reason? Have a specific method to export as much data as possible?
 - When account managers fail to save their held messages, they can end up with state that's potentially inconsistent - it might be wise to look at this and consider situations in which
 it makes the most sense to reload an account manager from its database JSON after a save failure. Of course this would have to come hand in hand with higher tolerance for possible
 insertion failures due to duplicates, otherwise we can end up with an account manager that reloads itself forever trying to save a message with a given ID.
 - Sort out our license, based on what we want and limitations from what we're already using (GPL...)
 
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
 
