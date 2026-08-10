#pragma once

#define __bdata __attribute__((section(".boot.data")))
#define __bcode __attribute__((section(".boot.text")))
