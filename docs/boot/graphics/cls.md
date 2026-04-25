# Overview

```c++
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL   *This
  );
```

```c++
SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
```

```c++
gST->ConOut->ClearScreen (gST->ConOut);
```
