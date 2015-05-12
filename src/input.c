// input.c

#include <IOKIT/hid/IOHIDKeys.h>
#include <IOKIT/hid/IOHIDManager.h>

#include "input.h"

#define internal static
#define global static
#define local static

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef float f32;
typedef double f64;

internal CFMutableDictionaryRef input_createDeviceMatchingDict(u32 usage_page, u32 usage) {
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                          0,
                                                          &kCFTypeDictionaryKeyCallBacks,
                                                          &kCFTypeDictionaryValueCallBacks);
  if (dict) {
    if (usage_page) {
      // add key for dive type to refine the matching dictionary
      CFNumberRef pageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page);
      if (pageNumberRef) {
        CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), pageNumberRef);
        CFRelease(pageNumberRef);

        // note: the usage is only valid if the usage page is also defined
        if (usage) {
          CFNumberRef usageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
          if (usageNumberRef) {
            CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usageNumberRef);
            CFRelease(usageNumberRef);
          } else {
            fprintf(stderr, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
          }
        }
      } else {
        fprintf(stderr, "%s: CFNumberCreate(usage_page) failed.", __PRETTY_FUNCTION__);
      }
    }
  } else {
    fprintf(stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
  }
  return dict;
}

internal void input_defaultDeviceMatchingCallback(void *context,
                                                  IOReturn result,
                                                  void *sender,
                                                  IOHIDDeviceRef device) {
  printf("%s(context: %p, result %p, sender: %p, device: %p).\n",
         __PRETTY_FUNCTION__, context, (void *)result, sender, (void *)device);
}

internal void input_defaultDeviceRemovalCallback(void *context,
                                                 IOReturn result,
                                                 void *sender,
                                                 IOHIDDeviceRef device) {
  printf("%s(context: %p, result %p, sender: %p, device: %p).\n",
         __PRETTY_FUNCTION__, context, (void *)result, sender, (void *)device);

}

internal void input_handleInputCallback(void *context,
                                        IOReturn result,
                                        void *sender,
                                        IOHIDValueRef value) {
  /* printf("%s(context: %p, result %p, sender: %p, value: %p).\n", */
  /*        __PRETTY_FUNCTION__, context, (void *)result, sender, (void *)value); */
  if (CFGetTypeID(value) != IOHIDValueGetTypeID()) {
    return;
  }

  CFIndex ival = IOHIDValueGetIntegerValue(value);
  IOHIDElementRef element = IOHIDValueGetElement(value);
  u32 type = IOHIDElementGetType(element);
  u32 page = IOHIDElementGetUsagePage(element);
  u32 use  = IOHIDElementGetUsage(element);


  printf("type: %d, page: %d, use: 0x%X, value: 0x%X\n", type, page, use, ival);
}

void input(void) {
  IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
  if (CFGetTypeID(hidManager) == IOHIDManagerGetTypeID()) {
    printf("IO HID Manager created.\n");

    CFDictionaryRef matchingDict = input_createDeviceMatchingDict(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
    if (matchingDict) {
      IOHIDManagerSetDeviceMatching(hidManager, matchingDict);

      IOHIDManagerRegisterDeviceMatchingCallback(hidManager, &input_defaultDeviceMatchingCallback, NULL);
      IOHIDManagerRegisterDeviceRemovalCallback(hidManager, &input_defaultDeviceRemovalCallback, NULL);

      IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

      if (IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone) == kIOReturnSuccess) {
        IOHIDManagerRegisterInputValueCallback(hidManager, &input_handleInputCallback, NULL);
      }
    } else {
      fprintf(stderr, "%s: input_createDeviceMatchingDict failed.", __PRETTY_FUNCTION__);
    }
  } else {
    printf("IO HID Manager not created.\n");
  }

  
}

