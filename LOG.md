#### 2022年4月5日 23点37分

1. 修复了ept的许多问题，添加了inline hook模块，不过暂未支持相对寻址的hook

2. 使用jmp相对跳转hook后，进入ept  violation read，给所有权限，然后开启MTF, 此后再进入MTF handler，按理说此时第一条jmp指令应该已经是被执行完毕了，可实际情况却并非如此，应该是还有其他问题。

3. 为找到并修复上述问题，现有如下想法

   + 详细阅读intel手册关于MTF的章节

   + 在ept violation里不开启MTF, 测试代码观察是否有问题

   + 参考其他源码的ept violation和mtf的处理方式


#### 2022年4月8日 23点36分

问题依旧，接下来应尝试

1. 关闭`interrupt window exiting`

2. 不进行ept权限控制，仅开启MTF, 观察指令行为
3. 实在不行，改push high, mov [rsp+4] ,low, ret 的方式

#### 2022年4月19日 21点06分

开启MTF后，出现了许多难以理解的现象，如

1. hv可能在ept violation handler处和MTF handler处反复横跳几次，最终进入proxy_hookedFunction。
2. hv可能在第一次进MTF前，越过了far jmp指令，来到了紧挨着的下一条指令。按理说MTF是不干扰程序的正常执行的，这里应该执行jmp语句，并跳转到指定的地址空间，可有时会越过这条jmp指令，直接来到它下面的那条指令，基本可以肯定此处触发了其他vmexit，然后使rip+=instruction length，之后再触发MTF。
3. hv可能一直ept violation handler处和MTF handler处横跳，进入MTF handler前，guest的第一条指令根本没有得到执行。

下次研究下DdiMon, 如果该项目方案有效，则借鉴一下

#### 2022年4月20日 22点35分

瞅了瞅DdiMon的源码后，发现自己原先的思路全然想错了。

原先的做法是在当因为read而导致EptViolation时，EptViolationHandler中将pte指向shadow page, 并给shadwo page读、写、执行的权限，然后开启MTF; 在MTFHandler中再还原original page以及在ept violation中给original page的权限，并关闭MTF。全然想错了。

正确的思路应该是，使用只需要执行权限的方式来进行hook，且original page始终保持只执行的权限，shadow页始终保持读、写、执行的权限。当发生EptViolation时，则必定是因为尝试读或写而导致的，此时应该直接将pte指向shadow page, 且赋予shadwo page读、写、执行的权限，然后清除EPT缓存，开启MTF。guest执行过一条指令后，中断到MtfHandler，将pte指向original page, 并赋予pte只执行的权限，然后清除EPT缓存，关闭MTF。

上述方案既可应付某代码页读取其他只执行页的情况（如PCHunter hook 检测），又可应对CRC检测（代码段完整性校验）。

