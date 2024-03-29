# 2019操作系统实验(四)

本次实验重点在于掌握：进程的概念、操作系统的系统调用、PV操作以及进程调度的实现。

## 1. 实现进程的调度

参考《Orange’s》第六章，在之前搭建的nasm+bochs实验平台上实现特定进程调度问题的模拟，

## 1.1 功能描述

- 添加系统调用接受int 型参数milli_seconds，调用此方法进程会在milli_seconds毫秒内不被分配时间片。
  - 注意，第六章代码已经在clock.c中有方法mills_delay，这个方法仍然为进程分配了时间片，只不过进程进入空循环。
- 添加系统调用打印字符串，接受char型参数str
  - 注意，第六章代码已经在kliba.asm文件中有了disp_str函数显示字符串，但这是内核函数。请实现并包装成相应的系统调用。
- 添加两个系统调用执行信号量PV 操作，在此基础上模拟读者写者问题。
  - 共有6个一直存在的进程（循环读写操作），A、B、C为读者进程，D、E为写者进程，F为普通进程，其中
    - A阅读消耗2个时间片
    - B、C阅读消耗3个时间片
    - D写消耗3个时间片
    - E写消耗4个时间片
  - 读者在读的时候，写者不能写，必须等到全部读者读完
  - 同时只能一个作者在写
  - 在写的时候，读者不能读
  - 多个读者可以读一本书，但是不能太多，上限数字有1、2、3，需要都能够支持，并且可以现场修改
  - A、B、C、D、E进程需要彩色打印基本操作：读开始、写开始、读、写、读完成、写完成，以及对应进程名字
  - F每隔1个时间片打印当前是读还是写，如果是读有多少人
  - 请分别实现读者优先和写者优先，需要都能够支持，并且可以现场修改
  - 请想办法解决此问题中部分情况下的进程饿死问题(可参考第六章)

### 1.2 注意事项

- 使用make 或类似工具构建整个项目。其中makefile 必须支持make run 命令直接启动，不需要其他命令。
- 本次作业可以直接在《orange’s》源代码基础上完成，请记录下添加或者修改的地方。
- 请提交代码、Makefile、说明文档和截图。

### 1.3 评分标准

完成上述全部要求可以获得全部分数
