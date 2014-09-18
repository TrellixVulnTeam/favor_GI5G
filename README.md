Just getting things set up right now. 

Todo (in order):
 - g++ 4.9 on Android? Necessary for <regex> implementations
 - Verify the shared_ptrs are acting the way I want them to for AccountManagers, because I'm new to this C++11 feature
 - Basic unit tests
 - Integrate rapidjson, use it to parse account details. We can probably just use it for our int->string conversions as well
 - Update the EmailManager compile flags so we can distinguish between compiling the local version, or whatever we'll need to support the android verison. Alternatively, just add conditionals inside 
 the EmailManager compilation (ugh) to check if we're on Android or not.
 - Start on threadsafety as described below
 
 
On Threading
==
 This may change over the course of development, but favor's current approach to threadsafety is a many-readers one-writer scheme. The worker:: namespace represents your writer, and is 
 _not designed to be threadsafe_. You can have as many threads calling ::reader namespace methods as you like, but there should be a designated worker thread (note that much of the ::worker logic may 
 end up being incidentally threadsafe in some cases because SQLite is threadsafe, but you are not encouraged to take chances. The reader needs very little locking as it is mostly about providing data,
 and maintains threadsafety as follows:
  - Important information is cached. This information can be retrieved, and also refreshed when changes to the database are made. In cases where we know exactly how a change to the database will
  adjust cached data (such as when we addd a new account), the data can be written to directly. In either case, refreshing/writing of any kind claims a writer lock. 
  - Locks are specific to the data they protect; generally, there's an "accounts" lock around the accounts data, and then a separate lock per account around that account's information in cache
  - The specifics depend on your application, but the reader is fast and gauranteed to be up-to-date. It is usually better to ask the reader for anything any time you need it than to store it yourself
  and risk stale data.
  - Depending on which std:: containers we use, there may or may not be a reader lock as well. This would exist only to keep the writer out while anyone was reading, and could potentially lead to
  writer starvation in cases with many reading threads, but this is not a use case Favor was designed for.
 

On Portability
==
Favor's c++ core is designed to be the backend for an eventual desktop program, as well asn an Android phone application. This means we want the _majority_ of Favor code to be portable to either
platform. However, realistically, there are some cases in which it serves us better to just rewrite code. Additionally, there are some MessageManagers that are decidedly platform specific. Examples:
 - Email will be available on both platforms, but almost certainly implemented differently (and largely in Java) on Android because of how difficult it has proven to get any kind of IMAP/MIME library
 working with the NDK.
 - Skype will be desktop only, because we rely on reading user's Skype logs which we won't have permission to do on Android (they might also be formatted differently, but this is moot if we can't read
 them at all).
 - Text messages will obviously be Android only, for obvious reasons. Also written largely in Java because Android has particularly convenient APIs for ensuring backwards compatibility which are not
 exposed in C++.
 - Line (and other thrift based clients) will be available on both platforms... and hopefully implemented similarly. This is a later part of the project, so we have only limited knowledge right now,
 but it seems there is some evidence of being able to compile thrift clients with the NDK based on Apache tickets here: https://issues.apache.org/jira/browse/THRIFT-1846. Also seems reasonable on the
 grounds that its main dependency is Boost, which has at least some (unofficial) NDK support.
 

Dependencies
==
 - Obviously, favor uses SQLite, but this is bundled and compiled with Favor itself (though this may eventually change so there's less superflous code in the repo). SQLite is an easy dependency to
   resolve.
 - The desktop implementation of the Email MessageManager (EmailManager) uses a [very slightly modified VMIME](https://github.com/Mindful/vmime/)  built using the command in 
 favor/src/lib/vmime/build_cmd. We'll move to using the official library as soon as it has support for IMAP search. Until then, the only changes in the homebrew version are to expose some 
 internals so we can run our own IMAP searches.