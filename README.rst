Overview
========

**Npemwin** is an emwin server. It was designed to get its feed from a
*Noaaport* server running *Nbsp*, but it can also be configured to get
the data from a standard byteblaster emwin server, a list of them,
or from an emwin serial device. In any case, it will turn serve
standard *byte-blaster emwin* clients in the usual way.Extensive
facilities allow to call a script for each complete file received
for post-processing (alarms/notifications, database insertion, etc).

**Npemwin** can used stand-alone or in conjuction with an Nbsp server.
**Npemwin** can connect to a list of *EMWIN* servers, or to a NOAAPort
Nbsp server or to a directly attached serial (*WX12/13*) device. It
saves all the files received in a directory based on file type and
*WFO ID*. Extensive facilities allow to call a script for each complete
file received for post-processing (alarms/notifications, database
insertion, and others). It can in turn serve any number of byte
blaster clients, including of course other instances of **Npemwin**
itself, and it can relay the data by ldm and nntp. **Npemwin** can be
used independently of *Nbsp*. In that mode, it can connect to any
number of byte blaster servers or to a directly attached emwin
serial (*WX12/13*) device. In any case, it will serve the data to any
number of byte blaster (BB) clients, including other instances of npemwin
itself, and it can relay the data by ldm and nntp.

It can be configured to register itself with the NWS master host
to be advertised as a *public emwin server* and appear in the list of
available servers. It has no hard-coded limits to the number of
clients it can serve. The number can be limited in the run-time
configuration file if desired. Clients can be dennied or allow
access through the tcpwrappers /etc/hosts.allow file. When used in
this way Npemwin tries to imitate ``bug for bug`` the behaviour of
a regular *BB* server, as seen by either the *BB* clients, the *BB* servers
or the master host.

It has a built-in web server for monitoring the internal state of
the server as well as displaying the files received using any web
browser.

What's New
==========

Sun Oct  7 18:34:46 AST 2012
----------------------------

Starting with version **2.3.1**, Npemwin supports reading from the
WX14 device ethernet data port. The *servers.conf* file
in the Npemwin configuration directory has the instructions
for enabling that configuration.
