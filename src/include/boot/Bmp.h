#ifndef __BMP_H__
#define __BMP_H__

#pragma pack(1)

typedef struct {
  UINT8   Blue;
  UINT8   Green;
  UINT8   Red;
  UINT8   Reserved;
} BMP_COLOR_MAP;

typedef struct {
  CHAR8         CharB;                        // BMP的标识符,必须为'B'
  CHAR8         CharM;                        // BMP的标识符,必须为'M'
  UINT32        Size;                         // BMP文件大小,Byte
  UINT16        Reserved1;                    // 保留
  UINT16        Reserved2;                    // 保留
  UINT32        ImageOffset;                  // BMP图像像素起始位置相对于头部的偏移
  UINT32        InfoHeaderSize;               // Bmp Info Header大小,40 B
  INT32         Width;                        // BMP图像宽
  INT32         Height;                       // BMP图像高
  UINT16        Planes;                       // 为 1
  UINT16        ImageBits;                    // BMP图像位深
  UINT32        CompressionType;              // 压缩方式
  UINT32        ImageSize;                    // 图像对齐时像素数据大小
  UINT32        HorizontalResolution;         // 横向分辨率
  UINT32        VerticalResolution;           // 纵向分辨率
  UINT32        PaletteColorsNum;             // 调色板颜色数
  UINT32        ImportantColorsNum;           // 重要颜色数
} BMP_IMAGE_HEADER;

#pragma pack()

typedef struct {
  BMP_IMAGE_HEADER Hdr;
  VOID*            Pixels;
  UINTN            Size;
} BMP_INFO;

EFI_STATUS
BmpInfoLoad (
  CHAR16   *Path,
  BMP_INFO *Bmp
  );

VOID
BmpInfoDestroy (
  BMP_INFO *Bmp
  );

#endif
