Just getting things set up right now. 

Todo (in order):
 - Make a serious attempt at getting Thrift C++ code and libcurl (ideally VMIME, but probably not) to compile with the NDK
 - Basic unit tests
 - Integrate rapidjson, use it to parse account details. We can probably just use it for our int->string conversions as well
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
  