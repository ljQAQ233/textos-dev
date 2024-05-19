# qemu-rtc

> 使用无所不能的 man ~

```shell
man qemu
```

```txt

-rtc [base=utc|localtime|datetime][,clock=host|rt|vm][,driftfix=none|slew]
      Specify  base  as utc or localtime to let the RTC start at the current UTC or local time, respectively.
      localtime is required for correct date in MS-DOS or Windows. To start at a specific point in time, pro‐
      vide datetime in the format 2006-06-17T16:01:21 or 2006-06-17. The default base is UTC.

      By default the RTC is driven by the host system time. This allows using of the RTC as  accurate  refer‐
      ence  clock  inside the guest, specifically if the host time is smoothly following an accurate external
      reference clock, e.g. via NTP. If you want to isolate the guest time from the host, you can  set  clock
      to  rt  instead, which provides a host monotonic clock if host support it. To even prevent the RTC from
      progressing during suspension, you can set clock to vm (virtual clock). 'clock=vm' is recommended espe‐
      cially in icount mode in order to preserve determinism; however, note that in icount mode the speed  of
      the virtual clock is variable and can in general differ from the host clock.

      Enable  driftfix  (i386 targets only) if you experience time drift problems, specifically with Windows'
      ACPI HAL. This option will try to figure out how many timer interrupts were not processed by  the  Win‐
      dows guest and will re-inject them.

```

