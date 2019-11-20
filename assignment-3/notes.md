# 试错笔记

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

## 保护模式

在保护模式下，虽然段值仍然由原来16位的cs,ds等寄存器表示，但此时它仅仅变成了一个索引，这个索引指向GDT表的表头，GDT表项即为描述符。　　
在设计特权级的每一步中，处理器都会对CPL、RPL、DPL等内容进行比较，这种比较无疑是动态的，是在运行过程中进行的，是发生在多个因素之间的行为。相对而言，段描述符中的界限、属性等内容则是静态的，是对某一项内容的界定和约束。

## 流程

```text
var:
    char_buffer
    buffer_size
    query_key
    key_len
    esc_flag
    upper_flag
    row_idx
    col_idx

function:
    print(char* addr, int ct)
        print ct chars starts stored in addr
    print_in_blue(char* addr, int ct)
        print ct chars starts stored in addr in blue
    clear()
        replace all video memory to zero
    undo()
        reduce buffer_size by 1 and recalculate idx
    find()
        find the key words in char_buffer and relpace them with blue ones
```

## loader

只要一个.com文件中不含有DOS系统调用，我们就可以把它当做Loader来使用。
