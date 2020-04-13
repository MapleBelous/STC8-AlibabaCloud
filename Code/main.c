#include "CloudHandle.h"
#include "Init.h"
void main(void)
{
    Init();
    while (true)
    {
        CloudLoop();
    }
}
