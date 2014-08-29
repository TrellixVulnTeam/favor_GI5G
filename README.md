Just getting things set up right now. 

Todo:
 - Verify builds on Android
 - Basic unit tests
 - Integrate rapidjson, use it to parse account details. We can probably just use it for our int->string conversions as well
 - Start on threadsafety as described below
 - Start in on exceptions, specifically verifying these on Android as well
 - Finalize at least the mail library so we can write a single message manager, if only for testing purposes
 
 
 On Threading
 ==
 This may change over the course of development, but favor's current approach to threadsafety is this: the worker:: namespace represents your writer, and is _not designed to be threadsafe_. You can have as many
 threads calling ::reader namespace methods as you like, but there should be a designated worker thread (note that much of the ::worker logic may end up being incidentally threadsafe because SQLite is
 threadsafe, but you are not encouraged to take chances. The reader needs very little locking as it is mostly about providing data, but because it maintains a cache the validity of which can change
 if the database is altered, it maintains threadsafety as follows:
  - Important information is cached. This information can be retrieved, and also refreshed when changes to the database are made.
  - This information is _never_ changed except by the method that refreshes it.
  - Under a many-readers-one-writer threading scheme, retrieving the information is (obviously) "reading" it and refreshing it is "writing" it.
  - Any number of readers can be reading at one time and the writer will wait until there are none left, but if the writer is writing, no one can read. In the abstract this could potentially starve
  our writer andm ake "refresh" operations block, but favor is not designed with that many threads in mind - if you need this kind of thing, you'll have to watch out for writer starvation yourself.
  - The specifics depend on your application, but the reader is fast and gauranteed to be up-to-date. It is usually better to ask the reader for anything any time you need it than to store it yourself
  and risk stale data.
