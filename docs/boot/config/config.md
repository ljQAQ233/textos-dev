# Overview

`Src/SigmaBootPkg/Config.c` 依赖于 `Src/SigmaBootPkg/Ini.c`

# Ini简单解析

我当初使用 **StdLib** 和 **iniparser-3.1**,但基于它的应用必须在Shell环境下启动运行,因为依赖.

于是就造了一个简单的轮子.(~~虽然不支持Section~~)

## Format

```ini
Key = Value # Comment
Key2 = Value2 ; Comment
```

# Config

使用 Config 封装.

