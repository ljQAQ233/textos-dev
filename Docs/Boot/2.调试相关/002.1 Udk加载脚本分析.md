# Automatical execution

每一次执行`n`或`s`后自动输出对应的C语言源代码,相应的Python实现代码在**script/udk_gdb_script**中`Edk2StopHandler`中对`stop-handler`的定义.

```python

class Edk2StopHandler(gdb.Command):
    """Command to be run when target is stopped.
       这些代码将在目标的被调试机器停止时执行.
    """
    def __init__(self):
        #向 Gdb 注册stop-handler命令.
        super(Edk2StopHandler, self).__init__("stop-handler", gdb.COMMAND_RUNNING, gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        args = UdkCommandHelper.checkParameter(arg, 0)
        if args == None:
            return
        gdb.execute("refresharch")
        if not UdkCommandHelper.supportExpat():
            gdb.execute("loadthis")
        gdb.execute("info exception")
Edk2StopHandler()

```

在停止时执行是靠如下命令设置的.

```cfg

define hook-stop
  stop-handler
end

```
