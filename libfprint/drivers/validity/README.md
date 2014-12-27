Validity Sensor
===============

This is not a complete solution as it requires a prepritory binary from Dell.  It does, however, allow you to use that finger print scanner that you paid for.

The following instructions were provided by [Pasquale 'sid' Fiorillo](http://blog.pasqualefiorillo.it/post/hp-probook-4540s-validity-sensors-driver-su-linux-mintdebian/)

What You'll need
----------------

Grab the Dell Finger Print driver, [Validity-Sensor-Setup-4.4-100.00.x86_64.rpm](ftp://ftp.hp.com/pub/softpaq/sp57001-57500/sp57317.tar), for HP ProBook 4540s for Linux 64-bit.  That's right, this only works for Linux 64-bit.  This worked for my laptop even though it did not fit the description nor was there a download option for this driver for my laptop.

Depending on your distribution, you will need the `alien` conversion tool for converting RPM to DEB.

    sudo apt-get install alien

The Validity Sensor Daemon may complain about two libraries missing.  Make sure these are installed by installing the correct ssl version,

    sudo apt-get install libssl0.9.8 

Some code has changed since this Pasquale's write up.  If your distro is slow to adopt code changes then you will need to build `fprintd` and `pam_fprint` yourself.

    git clone git://anongit.freedesktop.org/libfprint/fprintd
    git clone git://github.com/dsd/pam_fprint.git

Installing
----------

Convert the Validity Sensor package file, if needed.  And install,

    sudo alien -d Validity-Sensor-Setup-4.4-100.00.x86_64.rpm
    sudo dpkg -i validity-sensor-setup-4.4-100.00.x86_64.deb

Create a script for starting the Validity Fingerprint Service Daemon

    #!/bin/sh
    ### BEGIN INIT INFO
    # Provides: Validity Fingerprint Service Daemon
    # Required-Start: $remote_fs $syslog
    # Required-Stop: $remote_fs $syslog
    # Default-Start: 2 3 5
    # Default-Stop: 0 6
    # Short-Description: Validity Fingerprint Service Daemon
    # Description: Start vcsFPService to provide Validity FP Service
    ### END INIT INFO
    #
    # Note on runlevels:
    # 0 - halt/poweroff 6 - reboot
    # 1 - single user 2 - multiuser without network exported
    # 3 - multiuser w/ network (text mode) 5 - multiuser w/ network and X11 (xdm)
    #
    
        . /lib/lsb/init-functions
    
        vcsFPService_BIN=/usr/bin/vcsFPService
        test -x $vcsFPService_BIN || { echo "$vcsFPServiceDaemon is not installed";
        if [ "$1" = "stop" ]; then exit 0;
        else exit 5; fi; }
    
        case "$1" in
        start)
        echo -n "Starting vcsFPService Daemon \n"
        start-stop-daemon --start --background --quiet --exec "$vcsFPService_BIN" > /dev/null 2 >&1
    
        ;;
        stop)
        echo -n "Stopping vcsFPService Daemon \n"
    
    #/sbin/killall -TERM $vcsFPService_BIN
    start-stop-daemon --stop --quiet --quiet --oknodo --retry=0/1/KILL/5 --exec "$vcsFPService_BIN" > /dev/null 2 >&1
    
    ;;
    status)
    echo "Checking for vcsFPService Daemon \n"
    
    if pidof -o %PPID $vcsFPService_BIN> /dev/null; then
        echo "client is running. \n"
        exit 0
        else
            echo "client is not running. \n"
            exit 1
            fi
            ;;
            *)
            echo "Usage: $0 {start|stop|status}"
            exit 1
            ;;
            esac
    
            exit 0

Building Source Code
--------------------

At this point, I have provided source code for the Validity Sensor driver (this folder).  From the parent folder build as usual,

    ./autogen.sh
    make

If everything is to your liking then install,

    sudo make install


