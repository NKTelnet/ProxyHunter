#include "HttpRequest.hpp"

HttpRequest request[] =
{
   {"g-ecx.images-amazon.com",
    "images/G/01/x-locale/common/transparent-pixel._CB386942464_.gif",
    "image",
    43},

   {"static.licdn.com",
    "scds/common/u/img/pic/pic_profile_strength_mask_90x90_v2.png",
    "image",
    1085},
};

unsigned int MAXREQUEST = sizeof(request) / sizeof(HttpRequest);
