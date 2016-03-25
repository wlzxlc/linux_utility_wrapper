#include "videobuffer.h"
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define METHOD(inst, name) printf("GraphicBuffer:%s %p\n", #name, (inst)->name)

 typedef int (*GInterface)(OmxGraphics_t *);
 //IAndroid_CreateOmxGraphics(OmxGraphics_t *p);

int main(int c, char **s)
{
   if (c < 2) {
    printf("%s libs\n", s[0]);
    return 0;
   }

   const char *lib = s[1];

   printf("Open lib '%s'\n", lib);
   void *handle = dlopen(lib, RTLD_NOW);
   assert(handle);


   printf("Find symbol '%s'\n", "IAndroid_CreateOmxGraphics");
   GInterface createInst = (GInterface )dlsym(handle, "IAndroid_CreateOmxGraphics");
   assert(createInst);

   OmxGraphics_t api;

   printf("Find function '%p'\n", createInst);
   createInst(&api);

   METHOD(&api, alloc);       
   METHOD(&api, destory);
   METHOD(&api, width);     
   METHOD(&api, height);      
   METHOD(&api, stride);      
   METHOD(&api, usage);       
   METHOD(&api, pixelFormat);
   METHOD(&api, rect);
   METHOD(&api, handle);
   METHOD(&api, winbuffer);
   METHOD(&api, reallocate);
   METHOD(&api, lock);
   METHOD(&api, lockRect);
   METHOD(&api, unlock);
   METHOD(&api, lockYCbCr);
   METHOD(&api, lockYCbCrRect);

   graphics_handle *buffer = api.alloc(1920, 1080, IOMX_HAL_PIXEL_FORMAT_RGB_565, IOMX_GRALLOC_USAGE_HW_TEXTURE);
   assert(buffer);

   printf(" buffer %p w %d h %d stride %d pixfmt %d\n",
           buffer,
           (int)api.width(buffer), (int)api.height(buffer), (int)api.stride(buffer),(int) api.pixelFormat(buffer));

   void *addr = NULL;

   api.lock(buffer, IOMX_GRALLOC_USAGE_SW_WRITE_OFTEN, &addr);
   assert(addr);

   memset(addr, 0, 1920 * 1080 * 2);

   api.destory(buffer);

   return 0;
}
