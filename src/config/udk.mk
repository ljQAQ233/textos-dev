UDK_DBG_HOME := /opt/intel/udkdebugger
# the Root directory of Software UdkDebugger we have installed before
UDK_DBG_CFG  := /etc/udkdebugger.conf
# the Config file for udkdebugger

UDK_DBG_EXEC := $(UDK_DBG_HOME)/bin/udk-gdb-server
# the Executable file of UDK DEBUGGER

export UDK_DBG_HOME UDK_DBG_CFG UDK_DBG_EXEC
