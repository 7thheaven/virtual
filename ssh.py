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
res=''
success=False

paramiko.util.log_to_file('paramiko.log')
s=paramiko.SSHClient()

def execshow(cmd):
    global s,stdin,stdout,stderr,success,res
    success=False
    print cmd
    stdin,stdout,stderr=s.exec_command(cmd)
    #read means take away,need to save
    stderrstr=stderr.read()
    stdoutstr=stdout.read()
    if stderrstr:
        res+='stderr\n'+stderrstr
        print res
    else:
        print "stdout\n"+stdoutstr
        success=True
        res+=stdoutstr
    restext.SetValue(res)

def cleanmesg():
    global s,stdin,stdout,stderr,success
    success=False
    stdin,stdout,stderr=s.exec_command('dmesg -c')
    stderrstr=stderr.read()
    if stderrstr:
        print "stderr\n"+stderrstr
    else:
        success=True

def waitdone():
    global s,stdin,stdout,stderr,success
    success=False
    while not success:
        time.sleep(1)
        stdin,stdout,stderr=s.exec_command('cat /proc/jphyper/signal')
        stderrstr=stderr.read()
        stdoutstr=stdout.read()
        if stderrstr:
            print "stderr\n"+stderrstr
        else:
            print stdoutstr
            if stdoutstr == '0':
                print "Fin."
                success=True
                break

def simpletest():
    cleanmesg()
    execshow('insmod /home/jphyper/jphyper.ko')
    execshow('dmesg -c')
    execshow('/home/mvmfi 2 -1 17 0')
    execshow('dmesg -c')
    waitdone()
    execshow('dmesg -c')
    execshow('rmmod jphyper')
    execshow('dmesg -c')
    print 'All done.'

def cancel(event):
    s.close()

def bye(event):
    s.close()
    face.Close()

def connect():
    global s,hostname,username,password,res
    #s.load_system_host_keys()
    hostname=iptext.GetValue()
    username=unametext.GetValue()
    password=passwtext.GetValue()
    s.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    res+='Connecting to '+hostname
    restext.SetValue(res)
    s.connect(hostname = hostname,username=username, password=password)
    res+='Begin to Test'+hostname
    restext.SetValue(res)
    simpletest()

def connectasy(event):
    task=threading.Thread(target=connect)
    task.start()

sfi=wx.App()
face=wx.Frame(None,title="Virtual Fault Inject Platform",size=(800,600))
bkg=wx.Panel(face)
#ip,username,password--UI
iplabel=wx.StaticText(bkg,wx.NewId(),'IP :')
unamelabel=wx.StaticText(bkg,wx.NewId(),'UserName :')
passwlabel=wx.StaticText(bkg,wx.NewId(),'Password :')
iptext=wx.TextCtrl(bkg)
unametext=wx.TextCtrl(bkg)
passwtext=wx.TextCtrl(bkg)
iptext.SetValue('192.168.13.1')
unametext.SetValue('root')
passwtext.SetValue('secret')

restext=wx.TextCtrl(bkg,style=wx.TE_MULTILINE|wx.HSCROLL)
#switchfriend=wx.ListBox(bkg,-1,(0,0),(80,45),friendlist,wx.LB_SINGLE)

#bottom button--UI
conbutton=wx.Button(bkg,label='Connect')
conbutton.Bind(wx.EVT_BUTTON,connectasy)
cancelbutton=wx.Button(bkg,label='Cancel')
cancelbutton.Bind(wx.EVT_BUTTON,cancel)
exitbutton=wx.Button(bkg,label='Exit')
exitbutton.Bind(wx.EVT_BUTTON,bye)

#ip,username,password--layout
hbox1=wx.BoxSizer()
hbox2=wx.BoxSizer()
hbox3=wx.BoxSizer()
hbox1.Add(iplabel,proportion=1,flag=wx.EXPAND)
hbox1.Add(iptext,proportion=2,flag=wx.EXPAND)
hbox2.Add(unamelabel,proportion=1,flag=wx.EXPAND)
hbox2.Add(unametext,proportion=2,flag=wx.EXPAND)
hbox3.Add(passwlabel,proportion=1,flag=wx.EXPAND)
hbox3.Add(passwtext,proportion=2,flag=wx.EXPAND)
vbox1=wx.BoxSizer(wx.VERTICAL)
vbox1.Add(hbox1,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox1.Add(hbox2,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox1.Add(hbox3,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)

#bottom button--layout
hbox4=wx.BoxSizer()
hbox4.Add(conbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(cancelbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(exitbutton,proportion=1,flag=wx.EXPAND,border=5)

#All
vbox=wx.BoxSizer(wx.VERTICAL)
vbox.Add(vbox1,proportion=3,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(restext,proportion=6,flag=wx.EXPAND|wx.LEFT|wx.BOTTOM|wx.RIGHT,border=5)
vbox.Add(hbox4,proportion=1,flag=wx.EXPAND)
bkg.SetSizer(vbox)
face.Show()
sfi.MainLoop()
