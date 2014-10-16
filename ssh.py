#!/usr/bin/env python
import time,threading
import paramiko
import wx,wx.grid

hostname='192.168.13.1'
username='root'
password='secret'
stdin=''
stdout=''
stderr=''
success=False

paramiko.util.log_to_file('paramiko.log')
s=paramiko.SSHClient()
#s.load_system_host_keys()
s.set_missing_host_key_policy(paramiko.AutoAddPolicy())
s.connect(hostname = hostname,username=username, password=password)

def execshow(cmd):
    global s
    global stdin
    global stdout
    global stderr
    global success
    success=False
    print cmd
    stdin,stdout,stderr=s.exec_command(cmd)
    if stderr.read():
        print "stderr"
        print stderr.read()
    else:
        print "stdout"
        print stdout.read()
        success=True

def cleanmesg():
    global s
    global stdin
    global stdout
    global stderr
    global success
    success=False
    stdin,stdout,stderr=s.exec_command('dmesg -c')
    if stderr.read():
        print "stderr"
        print stderr.read()
    else:
        success=True

def waitdone():
    global s
    global stdin
    global stdout
    global stderr
    global success
    success=False
    while not success:
        time.sleep(1)
        stdin,stdout,stderr=s.exec_command('cat /proc/jphyper/signal')
        if stderr.read():
            print "stderr"
            print stderr.read()
        else:
            print "stdout"
            signal=stdout.read()
            print signal
            if signal == '0':
                print "Fin."
                success=True
                break

cleanmesg()
execshow('insmod /home/jphyper/jphyper.ko')
execshow('dmesg -c')
execshow('/home/mvmfi 2 -1 17 0')
execshow('dmesg -c')
waitdone()
execshow('dmesg -c')
execshow('rmmod jphyper')
execshow('dmesg -c')
s.close()
print 'All done.'
