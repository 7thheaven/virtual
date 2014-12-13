通过julyregfiguide.c经由gcc编译出的regfiguide文件来和内核模块julyregfi交互，完成寄存器故障注入实验。
其中内核模块的编译要确保编译环境系统版本和运行环境的一致。

使用说明：
直接运行regfiguide，不提供足够的命令行参数，则自动进入引导模式，可根据输出的提示进行故障模型的配置；
附带足够的命令行参数，可直接快速地进行传参和实验，格式如下
regfiguide aim fault time id
第一个是程序路径，aim是故障目标，fault是故障类型，time是注入次数，id目前没用到可能留作扩展。
例如：
/home/mayandong/regfiguide 3 4 10 0
表示对整个系统（3）的CX寄存器（4）进行10次（10）故障注入（目前是一位翻转故障）。
可以同时对多个寄存器注入故障，比如ax的fault代号为1，bx为2，cx为4，则fault输入7则表示三者全进行故障注入，6则是bx和cx，即是按位进行标记的。

一次完整的故障注入实验过程应该如下：
（1）部署阶段：
将julyregfiguide.c 拷贝到目录A下，如/home/mayandong/；
然后在该目录下编译——
gcc -o regfiguide julyregfiguide.c；
将内核模块部分，julyregfi.c及其相应的Makefile拷贝至目录B下，如/home/mayandong/julyregfi/；
然后在该目录下编译——
make；
（2）测试实验：
加载内核模块——
insmod /home/mayandong/julyregfi/julyregfi.ko
配置故障模型进行实验——
/home/mayandong/regfiguide 3 4 10 0
查看实验输出日志——（目前程序设置的前缀是Fortune，用以标示系统log中的该条信息是故障注入程序输出的）
dmesg
完成后卸载模块——
rmmod julyregfi
完毕。

其他问题：
编译内核模块失败，可能需要安装内核模块开发环境，kernel-devel kernel-headers等；
内核模块编译提示寄存器pt_regs错误，不同的系统和版本对该结构体内部成员的命名不一致，有的是ax，有的是eax，等等，但其内容和意义是一样的，因此无须管这个，拿到指针后，自己定义一个my_pt_regs，然后将该pt_regs直接用自己定义的类型强转解释使用即可，其他类似的问题（你有指针且明确知道该指针的内容的意义）也可这么解决。