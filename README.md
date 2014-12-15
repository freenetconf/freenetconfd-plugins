freenetconfd plugins
====================

The aim of this project is to provide plugins for _freenetconfd_ project
and plugin examples to get you started with writing of your own _freenetconfd_
plugins.

### Why _freenetconfd_ plugins?

NETCONF allows defining custom configuration and devices in a very potent way.
In order to enable you to take the full advantage of NETCONF in
_freenetconfd_, we had to provide you with the ability to define a behaviour
for your custom configurations or devices. That's where plugins come into play.

_freenetconfd-plugins_ are the home for popular _freenetconfd_ plugins, as
well as plenty of examples and documentation to kickstart you to writing
your own plugins.

### Benefits

Thanks to our code template generation and datastore implementation,
there is no need for you to deal with NETCONF calls, session control
or other implementation bits and pieces.
You can focus on implementing what's important for you, your get and
set functions, and rpc function calls. Go and light that bulb!

Some advanced options like `update()` and `set_multiple()` functions will
enable you to make some performance optimizations by reducing the number
of system calls required to fetch or set your data.

### Installation

#### Prerequisites
    freenetonfd

#### Installation
    git clone https://github.com/freenetconf/freenetconfd-plugins.git
    cd freenetconfd-plugins
    cd build
	cmake ..
	make
	make install
