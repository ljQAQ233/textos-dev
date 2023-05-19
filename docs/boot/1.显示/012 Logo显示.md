# Resources

在 `Src/Resources` 下的文件将会被复制到镜像根目录.

- Sigma.bmp -> Sigma1.bmp
- Sigma1.bmp -> 1 bit
- Sigma4.bmp -> 4 bit
- Sigma8.bmp -> 8 bit
- Sigma24.bmp -> 24 bit

# GraphicsBmpDisplay

```c++
EFI_STATUS
GraphicsBmpDisplay (
  IN CHAR16 *Path,
  IN UINT64 X,
  IN UINT64 Y,
  IN UINT64 Mode
  );
```

`Mode` 设置显示位置.

## 显示位置

```c++
enum BmpDisplayMode {
  ModeNormal    = 1,
  ModeHorMiddle = (1 << 1),
  ModeVerMiddle = (1 << 2),
  ModeTop       = (1 << 3),
  ModeLeft      = (1 << 4),
  ModeBottom    = (1 << 5),
  ModeRight     = (1 << 6),
  ModeCenter    = (1 << 7),
};
```

# LogoShow

对 `GraphicsBmpDisplay` 进行封装.

