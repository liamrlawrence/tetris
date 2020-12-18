#include <stdlib.h>
#include "tetris.h"

int main()
{
    srand(0);

    tetris.init();
    tetris.run();
    tetris.close();
}
