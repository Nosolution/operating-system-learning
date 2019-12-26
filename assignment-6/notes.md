# 项目笔记

## 项目实现

大部分借鉴(chaoxi)了Orange's第五章的代码。主要修改部分：

- tty.h,tty.c，其中修改了大部分的逻辑以完成作业
- keyboard.c，因被原tty的逻辑涉及而被一同修改
- proto.h，修改和增加函数原形
- proc.h，修改一些进程相关的参数，去除了与作业无关的进程

## 使用

1. make image
2. make run

## bochs上安装FreeDos

1. 下载[FreeDos镜像](http://bochs.sourceforge.net/guestos/freedos-img.tar.gz)，解压复制**a.img**到工作目录并改名为freedos.img。至于如何使用FreeDos1.2还有待研究。
2. 使用bximage命令新建软盘，重命名为pm.img。
3. 修改bochs配置文件bochsrc，修改相应代码段为：  

    ```text
    # what disk images will be used 
    floppya: 1_44=freedos.img, status=inserted
    floppyb: 1_44=pm.img, status=inserted

    # choose the boot disk.
    boot: a
    ```

4. 启动bochs，输入｀format b｀格式化b盘，退出bochs
5. 编译代码: `nasm pmtest1.asm -o pmtest1.com`为可执行文件
6. 顺序输入命令以传入代码:

    ```text
    sudo mount -o loop pm.img /mnt/floppy/
    sudo cp pmtest1.com /mnt/floppy/
    sudo umount /mnt/floppy
    ```

7. 再次启动bochs，转到b盘符，输入`pmtest1.com`

## loader

只要一个.com文件中不含有DOS系统调用，我们就可以把它当做Loader来使用。
