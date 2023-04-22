# author : Maouai233
# date   : 2023/03/25
# description : 真没想到老子的udk脚本在 MIDebugger 初始化完成后被抛弃了......

# 此脚本已被遗弃，最初用于解决 setupCommands 后加载的函数会被抹去的问题，
# 但现在使用 customLaunchSetupCommands 全部启动命令由我们控制

define hook-stop
  # 在 MIDebugger 初始化完成后加载 udk 脚本
  # hook-stop 在此脚本中被重新设置为 loadthis
  # 在此过程后,此 hook-stop 将被遗置
  source /opt/intel/udkdebugger/script/udk_gdb_script
  loadthis
end
