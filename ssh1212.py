#!/usr/bin/env python
import time,threading,os
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
fitoolsdir='./fitools'
fitoolslist=''

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
    res+='Connecting to '+hostname+'\n'
    restext.SetValue(res)
    s.connect(hostname = hostname,username=username, password=password)
    res+='Begin to Test'+hostname+'\n'
    restext.SetValue(res)
    simpletest()

def connectasy(event):
    task=threading.Thread(target=connect)
    task.start()

def modelshow(event):
    modelface.Show()

def modelexit(event):
    modelface.Hide()

def modelnext(event):
    pass

def fitoolsinit():
    global fitoolsdir,fitoolslist
    fitoolslist=os.listdir(fitoolsdir)
    print fitoolslist

sfi=wx.App()
face=wx.Frame(None,title="Virtual Fault Inject Platform",size=(800,600))
bkg=wx.Panel(face)
#ip,username,password--UI
sshlabel=wx.StaticText(bkg,wx.NewId(),'Connection',(0,0),(0,0),wx.ALIGN_CENTER)
iplabel=wx.StaticText(bkg,wx.NewId(),'IP :')
unamelabel=wx.StaticText(bkg,wx.NewId(),'UserName :')
passwlabel=wx.StaticText(bkg,wx.NewId(),'Password :')
iptext=wx.TextCtrl(bkg)
unametext=wx.TextCtrl(bkg)
passwtext=wx.TextCtrl(bkg,style=wx.TE_PASSWORD)
iptext.SetValue('192.168.13.1')
unametext.SetValue('root')
passwtext.SetValue('secret')

#model select -- UI
modelbutton=wx.Button(bkg,label='Select Fault-Model')
modelbutton.Bind(wx.EVT_BUTTON,modelshow)
#statelabel=wx.StaticText(bkg,wx.NewId(),'State',(0,0),(0,0),wx.ALIGN_CENTER)
statetext=wx.TextCtrl(bkg,style=wx.TE_MULTILINE|wx.HSCROLL|wx.TE_READONLY)

#result show
reslabel=wx.StaticText(bkg,wx.NewId(),'Log',(0,0),(0,0),wx.ALIGN_CENTER)
restext=wx.TextCtrl(bkg,style=wx.TE_MULTILINE|wx.HSCROLL)

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
vbox1.Add(sshlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox1.Add(hbox1,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox1.Add(hbox2,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox1.Add(hbox3,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)

#model select -- layout
hbox5=wx.BoxSizer()
hbox5.Add(modelbutton,proportion=2,flag=wx.EXPAND,border=5)
#hbox5.Add(statelabel,proportion=1,flag=wx.EXPAND,border=3)
hbox5.Add(statetext,proportion=3,flag=wx.EXPAND,border=5)

#bottom button--layout
hbox4=wx.BoxSizer()
hbox4.Add(conbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(cancelbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(exitbutton,proportion=1,flag=wx.EXPAND,border=5)

#All
vbox=wx.BoxSizer(wx.VERTICAL)
vbox.Add(vbox1,proportion=2,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(hbox5,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(reslabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(restext,proportion=6,flag=wx.EXPAND|wx.LEFT|wx.BOTTOM|wx.RIGHT,border=5)
vbox.Add(hbox4,proportion=1,flag=wx.EXPAND)
bkg.SetSizer(vbox)
fitoolsinit()

#model select frame
modelface=wx.Frame(face,title="Fault-Model Selection",size=(400,300))
modelbkg=wx.Panel(modelface)
modeltext=wx.StaticText(modelbkg,wx.NewId(),'Aim',(0,0),(0,0),wx.ALIGN_CENTER)
modelswitch=wx.ListBox(modelbkg,-1,(0,0),(0,0),fitoolslist,wx.LB_SINGLE)
#modelswitch.Set(fitoolslist)
modelswitch.SetSelection(0,True)
modelnextbutton=wx.Button(modelbkg,label='Next')
modelnextbutton.Bind(wx.EVT_BUTTON,modelnext)
modelexitbutton=wx.Button(modelbkg,label='Exit')
modelexitbutton.Bind(wx.EVT_BUTTON,modelexit)
hboxmodel=wx.BoxSizer()
hboxmodel.Add(modelnextbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxmodel.Add(modelexitbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
vboxmodel=wx.BoxSizer(wx.VERTICAL)
vboxmodel.Add(modeltext,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
vboxmodel.Add(modelswitch,proportion=8,flag=wx.EXPAND|wx.ALL,border=5)
vboxmodel.Add(hboxmodel,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
modelbkg.SetSizer(vboxmodel)


face.Show()
sfi.MainLoop()
