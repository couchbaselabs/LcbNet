========================================================================
    DYNAMIC LINK LIBRARY : LCouchbase Project Overview
========================================================================

The LCouchbase library is an experimental implementation of
a Couchbase client using the C client library (libcouchbase) and binding it
to the .NET runtime (CLR) using C++/CLI.

Currently, Get and Store (along with append, prepend, etc) are implemented
It is intended that this API would be wrapped by another C# DLL.

TODO:
	* Implement command futures
	* Make Couchbase run in a background thread and receive
		operations in a queue. This can be used in an async fashion
	* Implement wrapper interface (this should be similar to the official SDK)
	* Probably clean up this project

If you'd like more information on how to use, or if you have a bug, please
drop by #libcouchbase on irc.freenode.net

Mark Nunberg