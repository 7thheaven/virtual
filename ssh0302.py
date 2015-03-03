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
fitoolscmds={}
cmdlist=['initcmd','guidepath','runningcmd','donecmd']
living=True

paramiko.util.log_to_file('paramiko.log')
s=paramiko.SSHClient()

def getdate():
    return time.strftime('%Y-%m-%d-%H-%M',time.localtime(time.time()))

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

def waitdone(fitoolname):
    global s,stdin,stdout,stderr
    success=False
    if fitoolname=='cpufi':
        path='julyregfi'
    elif fitoolname=='xenfi':
        path='jphyper'
    while not success:
        time.sleep(1)
        stdin,stdout,stderr=s.exec_command('cat /proc/'+path+'/signal')
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
    res+='Connected.\n'
    restext.SetValue(res)
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
            cpuuse=stdoutstr.split()[0]
            if float(cpuuse)>100:
                cpuuse='100'
            cpulabel.SetLabel('CPU: '+cpuuse+'%')
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
    waitdone(fitool)
    for cmd in fitoolscmds[fitool]['donecmd']:
        execshow(cmd)
    print 'All done.'

def getlog(event):
    execshow('cat /home/log/out.txt')
    filog=open('./log/log'+getdate(),'w')
    filog.write(res)
#print res

def clean(event):
    global res
    res=''
    restext.SetValue(res)

def fitestasy(event):
    task=threading.Thread(target=fitest)
    task.start()

def modelshow(event):
    global fitoolslist,fitoolchoice
    fitool=fitoolslist[fitoolchoice.GetSelection()]
    if fitool=='xenfi':
        xenfiface.Show()
    elif fitool=='cpufi':
        cpufiface.Show()

def modelexit(event):
    global fitoolslist,fitoolchoice
    fitool=fitoolslist[fitoolchoice.GetSelection()]
    if fitool=='xenfi':
        xenfiface.Hide()
    elif fitool=='cpufi':
        cpufiface.Hide()

def xenfiselect(event):
    global xenfiaimchoice,xenfifaultchoice,xenfitimetext,xenfiidtext,argstext
    aim=xenfiaimchoice.GetSelection()
    fault=xenfifaultchoice.GetSelection()-1
    time=xenfitimetext.GetValue()
    id=xenfiidtext.GetValue()
    argstext.SetValue(str(aim)+' '+str(fault)+' '+time+' '+id)
    modelexit(event)

def xenfiaimchange(event):
    global xenfiaimchoice,xenfifaultchoice,xenfifault
    xenfifaultchoice.Set(xenfifault[xenfiaimchoice.GetSelection()])
    xenfifaultchoice.SetSelection(0)

def cpufiselect(event):
    global cpufiaimchoice,cpufifaultbox,cpufitimetext,argstext,cpufifault
    aim=cpufiaimchoice.GetSelection()
    l=len(cpufifault)
    fault=0
    for i in range(0,l):
        if cpufifaultbox.IsChecked(i):
            fault^=1<<i
    time=cpufitimetext.GetValue()
    argstext.SetValue(str(aim)+' '+str(fault)+' '+time)
    modelexit(event)

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
modelbutton=wx.Button(bkg,label='Select Fault-Model')
modelbutton.Bind(wx.EVT_BUTTON,modelshow)
argslabel=wx.StaticText(bkg,wx.NewId(),'args:',(0,0),(0,0),wx.ALIGN_CENTER)
argstext=wx.TextCtrl(bkg,style=wx.TE_READONLY)

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
cleanbutton=wx.Button(bkg,label='Clean')
cleanbutton.Bind(wx.EVT_BUTTON,clean)
logbutton=wx.Button(bkg,label='ShowLog')
logbutton.Bind(wx.EVT_BUTTON,getlog)
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
hbox5.Add(fitoolchoice,proportion=4,flag=wx.EXPAND,border=5)
hbox5.Add(modelbutton,proportion=4,flag=wx.EXPAND,border=5)
hbox5.Add(argslabel,proportion=1,flag=wx.EXPAND,border=1)
hbox5.Add(argstext,proportion=4,flag=wx.EXPAND,border=5)

#bottom button--layout
hbox4=wx.BoxSizer()
hbox4.Add(conbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(fibutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(cancelbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(cleanbutton,proportion=1,flag=wx.EXPAND,border=5)
hbox4.Add(logbutton,proportion=1,flag=wx.EXPAND,border=5)
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

#model select frame--xenfi
xenfiaim=('ioctl_privcmd_hypercall','xen_pgd_pin','xen_l2_entry_update','remap_pfn_range','xen_l3_entry_update','rw_block_io')
xenfifault=(('none','cmd','mfn','linear_addr'),('none','cmd','mfn'),('none','ptr_address','ptr','val'),('none','pgd','vm_flags'),('none','ptr_address','ptr','val'),('none','id','nr_pages','nr_segments'))
#cpuaim=('Fork Reg_FI','Scan Reg_FI','Both')
xenfiface=wx.Frame(face,title="Fault-Model Selection",size=(400,300))
xenfibkg=wx.Panel(xenfiface)
xenfiaimlabel=wx.StaticText(xenfibkg,wx.NewId(),'Aim:',(0,0),(0,0),wx.ALIGN_CENTER)
xenfiaimchoice=wx.Choice(xenfibkg,wx.NewId(),(0,0),choices=xenfiaim)
xenfiaimchoice.SetSelection(0)
xenfiaimchoice.Bind(wx.EVT_CHOICE,xenfiaimchange)
xenfifaultlabel=wx.StaticText(xenfibkg,wx.NewId(),'Fault:',(0,0),(0,0),wx.ALIGN_CENTER)
xenfifaultchoice=wx.Choice(xenfibkg,wx.NewId(),(0,0),choices=xenfifault[0])
xenfifaultchoice.SetSelection(0)
xenfitimelabel=wx.StaticText(xenfibkg,wx.NewId(),'Time:',(0,0),(0,0),wx.ALIGN_CENTER)
xenfitimetext=wx.TextCtrl(xenfibkg)
xenfiidlabel=wx.StaticText(xenfibkg,wx.NewId(),'Id:',(0,0),(0,0),wx.ALIGN_CENTER)
xenfiidtext=wx.TextCtrl(xenfibkg)
hboxxenfiaim=wx.BoxSizer()
hboxxenfiaim.Add(xenfiaimlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfiaim.Add(xenfiaimchoice,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfifault=wx.BoxSizer()
hboxxenfifault.Add(xenfifaultlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfifault.Add(xenfifaultchoice,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfitime=wx.BoxSizer()
hboxxenfitime.Add(xenfitimelabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfitime.Add(xenfitimetext,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfiid=wx.BoxSizer()
hboxxenfiid.Add(xenfiidlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfiid.Add(xenfiidtext,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
xenfiselectbutton=wx.Button(xenfibkg,label='Select')
xenfiselectbutton.Bind(wx.EVT_BUTTON,xenfiselect)
xenfiexitbutton=wx.Button(xenfibkg,label='Exit')
xenfiexitbutton.Bind(wx.EVT_BUTTON,modelexit)
hboxxenfi=wx.BoxSizer()
hboxxenfi.Add(xenfiselectbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxxenfi.Add(xenfiexitbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
vboxxenfi=wx.BoxSizer(wx.VERTICAL)
vboxxenfi.Add(hboxxenfiaim,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxxenfi.Add(hboxxenfifault,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxxenfi.Add(hboxxenfitime,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxxenfi.Add(hboxxenfiid,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxxenfi.Add(hboxxenfi,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
xenfibkg.SetSizer(vboxxenfi)

#model select frame--cpufi
cpufiaim=('Fork Reg_FI','Scan Reg_FI','Both')
cpufifault=('ax','bx','cx','dx','si','di','bp','ds','es','fs','gs','orig_ax','ip','cs','flags','sp','ss')
cpufiface=wx.Frame(face,title="Fault-Model Selection",size=(400,300))
cpufibkg=wx.Panel(cpufiface)
cpufiaimlabel=wx.StaticText(cpufibkg,wx.NewId(),'Aim:',(0,0),(0,0),wx.ALIGN_CENTER)
cpufiaimchoice=wx.Choice(cpufibkg,wx.NewId(),(0,0),choices=cpufiaim)
cpufiaimchoice.SetSelection(2)
cpufifaultlabel=wx.StaticText(cpufibkg,wx.NewId(),'Fault:',(0,0),(0,0),wx.ALIGN_CENTER)
cpufifaultbox=wx.CheckListBox(cpufibkg,-1,(0,0),(0,0),cpufifault)
cpufifaultbox.Check(2,True)
cpufitimelabel=wx.StaticText(cpufibkg,wx.NewId(),'Time:',(0,0),(0,0),wx.ALIGN_CENTER)
cpufitimetext=wx.TextCtrl(cpufibkg)
hboxcpufiaim=wx.BoxSizer()
hboxcpufiaim.Add(cpufiaimlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufiaim.Add(cpufiaimchoice,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufifault=wx.BoxSizer()
hboxcpufifault.Add(cpufifaultlabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufifault.Add(cpufifaultbox,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufitime=wx.BoxSizer()
hboxcpufitime.Add(cpufitimelabel,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufitime.Add(cpufitimetext,proportion=4,flag=wx.EXPAND|wx.ALL,border=2)
cpufiselectbutton=wx.Button(cpufibkg,label='Select')
cpufiselectbutton.Bind(wx.EVT_BUTTON,cpufiselect)
cpufiexitbutton=wx.Button(cpufibkg,label='Exit')
cpufiexitbutton.Bind(wx.EVT_BUTTON,modelexit)
hboxcpufi=wx.BoxSizer()
hboxcpufi.Add(cpufiselectbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
hboxcpufi.Add(cpufiexitbutton,proportion=1,flag=wx.EXPAND|wx.ALL,border=2)
vboxcpufi=wx.BoxSizer(wx.VERTICAL)
vboxcpufi.Add(hboxcpufiaim,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxcpufi.Add(hboxcpufifault,proportion=4,flag=wx.EXPAND|wx.ALL,border=5)
vboxcpufi.Add(hboxcpufitime,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
vboxcpufi.Add(hboxcpufi,proportion=1,flag=wx.EXPAND|wx.ALL,border=5)
cpufibkg.SetSizer(vboxcpufi)

face.Show()
sfi.MainLoop()
