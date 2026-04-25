# Makefile 组织

- `src/config` -> 配置文件
- `src/config/main.mk` -> 主配置,私人配置,sudo密码等.

---

- `src/Makkefile` -> 顶层 makfile

- `make`
  - 加载配置文件
  - 编译boot模块
    - 检测build目录是否包含最新编译版本
      - YES -> Skip
      - NO  -> Continue
        - `make`
          - ... -> 有锁的循环调用

---

- `src/boot/Makefile` -> boot 模块 makefile

对于一个编译行为,应调用 `(make )update` 来进行,它检测项目改动,如果有改动,则通过顶层makefile来调用`(make )build`.

