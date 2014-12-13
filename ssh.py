#!/usr/bin/env python
import time,threading,os,commands
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
fitoolscmds={}
cmdlist=['initcmd','guidepath','runningcmd','donecmd']
statelabel=''
cpulabel=''
memorylabel=''
living=True
fitoolchoice=''
argstext=''

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
    living=False
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
    s.connect(hostname=hostname,username=username, password=password)
    refreshstatusasy()
#res+='Begin to Test'+hostname+'\n'
#restext.SetValue(res)
#simpletest()

def connectasy(event):
    task=threading.Thread(target=connect)
    task.start()

def fitoolsinit():
    global fitoolsdir,fitoolslist,fitoolscmds,cmdlist
    fitoolslist=os.listdir(fitoolsdir)
    #print fitoolslist
    for fitool in fitoolslist:
        fitoolscmds[fitool]={}
        ficmd=open(fitoolsdir+'/'+fitool,'r')
        cmd=ficmd.read().split('\n')
        temp='err'
        s=[]
        for i in cmd:
            if i in cmdlist:
                if temp!='err' and len(s)>0:
                    fitoolscmds[fitool][temp]=s
                temp=i
                s=[]
            else:
                s.append(i)
        if temp!='err' and len(s)>0:
            fitoolscmds[fitool][temp]=s
#print fitoolscmds

def setconnect(flag):
    global statelabel
    if flag:
        statelabel.SetLabel('Connect')
        statelabel.SetForegroundColour(wx.GREEN)
    else:
        statelabel.SetLabel('Disconnect')
        statelabel.SetForegroundColour(wx.RED)

def refreshstatus():
    global s,cpulabel,memorylabel,living
    STATS = []
    while living:
        mem = {}
        success=False
        stdin,stdout,stderr=s.exec_command('cat /proc/meminfo')
        #read means take away,need to save
        stderrstr=stderr.read()
        stdoutstr=stdout.readlines()
        if stderrstr:
            setconnect(False)
            print stderrstr
        else:
            setconnect(True)
            success=True
            lines=stdoutstr
            for line in lines:
                name = line.split(':')[0]
                var = line.split(':')[1].split()[0]
                mem[name] = float(var)
            STATS[0:] = [mem['MemTotal']]
            mem['MemUsed'] = mem['MemTotal'] - mem['MemFree'] - mem['Buffers'] - mem['Cached']
            STATS[1:] = [mem['MemUsed']]
            u = round((mem['MemUsed'])/(mem['MemTotal']),5)
            STATS.append('%.2f%%'%(u*100))
            MemT = STATS[0]
            MemU = STATS[1]
            usedper = STATS[2]
            memorylabel.SetLabel('Mem: '+str(usedper))
        success=False
        if not living:
            break
        stdin,stdout,stderr=s.exec_command('ps -eo pcpu | awk \'{if(NR!=1){sum+=$1}}END{print sum}\'')
        #read means take away,need to save
        stderrstr=stderr.read()
        stdoutstr=stdout.read()
        if stderrstr:
            setconnect(False)
            print stderrstr
        else:
            setconnect(True)
            success=True
            cpulabel.SetLabel('CPU: '+stdoutstr.split()[0]+'%')
        time.sleep(1)


def refreshstatusasy():
    task=threading.Thread(target=refreshstatus)
    task.start()

def fitest():
    global fitoolslist,fitoolscmds,fitoolchoice,argstext
    fitool=fitoolslist[fitoolchoice.GetSelection()]
    args=argstext.GetValue()
    #print fitoolscmds[fitool]
    cleanmesg()
    for cmd in fitoolscmds[fitool]['initcmd']:
        execshow(cmd)
    execshow(fitoolscmds[fitool]['guidepath'][0]+' '+args)
    for cmd in fitoolscmds[fitool]['runningcmd']:
        execshow(cmd)
    waitdone()
    for cmd in fitoolscmds[fitool]['donecmd']:
        execshow(cmd)
    print 'All done.'

def fitestasy(event):
    task=threading.Thread(target=fitest)
    task.start()

sfi=wx.App()
face=wx.Frame(None,title="Virtual Fault Inject Platform",size=(800,600))
bkg=wx.Panel(face)

#status showing--UI
statuslabel=wx.StaticText(bkg,wx.NewId(),'Status',(0,0),(0,0),wx.ALIGN_CENTER)
statelabel=wx.StaticText(bkg,wx.NewId(),'Disconnect',(0,0),(0,0),wx.ALIGN_CENTER|wx.ST_NO_AUTORESIZE)
statelabel.SetForegroundColour(wx.RED)
cpulabel=wx.StaticText(bkg,wx.NewId(),'CPU: ?%',(0,0),(0,0),wx.ALIGN_CENTER|wx.ST_NO_AUTORESIZE)
memorylabel=wx.StaticText(bkg,wx.NewId(),'Mem: ?%',(0,0),(0,0),wx.ALIGN_CENTER|wx.ST_NO_AUTORESIZE)

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

#fitools select -- UI
fitoolsinit()
fitoollabel=wx.StaticText(bkg,wx.NewId(),'FI_Tool:',(0,0),(0,0),wx.ALIGN_CENTER)
fitoolchoice=wx.Choice(bkg,wx.NewId(),(0,0),choices=fitoolslist)
fitoolchoice.SetSelection(0)
argslabel=wx.StaticText(bkg,wx.NewId(),'args:',(0,0),(0,0),wx.ALIGN_CENTER)
argstext=wx.TextCtrl(bkg)
#statetext=wx.TextCtrl(bkg,style=wx.TE_MULTILINE|wx.HSCROLL|wx.TE_READONLY)

#result show
reslabel=wx.StaticText(bkg,wx.NewId(),'Log',(0,0),(0,0),wx.ALIGN_CENTER)
restext=wx.TextCtrl(bkg,style=wx.TE_MULTILINE|wx.HSCROLL)

#bottom button--UI
conbutton=wx.Button(bkg,label='Connect')
conbutton.Bind(wx.EVT_BUTTON,connectasy)
fibutton=wx.Button(bkg,label='FI_Test')
fibutton.Bind(wx.EVT_BUTTON,fitestasy)
cancelbutton=wx.Button(bkg,label='Cancel')
cancelbutton.Bind(wx.EVT_BUTTON,cancel)
exitbutton=wx.Button(bkg,label='Exit')
exitbutton.Bind(wx.EVT_BUTTON,bye)

#status showing--layout
vbox2=wx.BoxSizer(wx.VERTICAL)
vbox2.Add(statuslabel,proportion=1,flag=wx.EXPAND)
vbox2.Add(statelabel,proportion=1,flag=wx.EXPAND)
vbox2.Add(cpulabel,proportion=1,flag=wx.EXPAND)
vbox2.Add(memorylabel,proportion=1,flag=wx.EXPAND)

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

#fitools select -- layout
hbox5=wx.BoxSizer()
hbox5.Add(fitoollabel,proportion=1,flag=wx.EXPAND,border=1)
hbox5.Add(fitoolchoice,proportion=3,flag=wx.EXPAND,border=5)
hbox5.Add(argslabel,proportion=1,flag=wx.EXPAND,border=1)
hbox5.Add(argstext,proportion=3,flag=wx.EXPAND,border=5)

#bottom button--layout
hbox4=wx.BoxSizer()
hbox4.Add(conbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(fibutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(cancelbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(exitbutton,proportion=1,flag=wx.EXPAND,border=5)

#All
vbox=wx.BoxSizer(wx.VERTICAL)
hbox6=wx.BoxSizer()
hbox6.Add(vbox2,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
hbox6.Add(vbox1,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(hbox6,proportion=3,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(hbox5,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(reslabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vbox.Add(restext,proportion=7,flag=wx.EXPAND|wx.LEFT|wx.BOTTOM|wx.RIGHT,border=5)
vbox.Add(hbox4,proportion=1,flag=wx.EXPAND)
bkg.SetSizer(vbox)

face.Show()
sfi.MainLoop()
