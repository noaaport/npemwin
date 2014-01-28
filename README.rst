Overview
========

**Npemwin** is an emwin server. It was designed to get its feed from a
an *Nbsp* *Noaaport* server, but it can also be configured to get
the data from a standard byteblaster emwin server, a list of them,
from an emwin serial device (WX12/13/14) or from the newer WX14 device.
In any case, it will turn serve standard *byte-blaster emwin* clients
in the usual way.

All the files received are saved in a directory based on file type and
*WFO ID*. Extensive facilities allow to call a script for each complete
file received for post-processing (alarms/notifications, database
insertion, and others). It can in turn serve any number of byte
blaster clients, including of course other instances of **Npemwin**
itself, and it can relay the data by ldm and nntp.

It can be configured to register itself with the NWS master host
to be advertised as a *public emwin server* and appear in the list of
available servers. It has no hard-coded limits to the number of
clients it can serve. The number can be limited in the run-time
configuration file if desired. Clients can be dennied or allow
access through the tcpwrappers the */etc/hosts.allow* file. When used in
this way Npemwin tries to imitate the behaviour of
a regular *BB* server, as seen by either the *BB* clients, the *BB* servers
or the master host.

It has a built-in web server for monitoring the internal state of
the server as well as displaying the files received using any web
browser.

Binary packages are available from

  * http://www.noaaport.net
  * http://bb.noaaport.net/npemwin/downloads

What's New
==========

Fri Oct 25 08:34:29 AST 2013
---------------------------

An android application **EmwinMobi** is available for beta testing
from

  * ftp://www.noaaport.net/software

Tue Feb  5 13:46:46 AST 2013
----------------------------

Starting with version **2.4.1** Npemwin has a *mobile* web interface for better
display in mobile devices, e.g., smartphones and tablets,
and a revised NNTP interface. More information in

  * http://emwin.mobi
  * http://www.iemwin.net
  * http://bb.noaaport.net/npemwin-nntp

Sun Oct  7 18:34:46 AST 2012
----------------------------

Starting with version **2.3.1**, Npemwin supports reading from the
WX14 device ethernet ports. The *servers.conf* file
in the Npemwin configuration directory has the instructions
for enabling that configuration.
