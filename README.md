# Interprocess-Communication
Processor farm to create a Mandelbrot Fractal Image.

Farmer creates child processes which compute the one line of the image. The farmer then collects the data from the children.

Communication is done using messege queues.

Compiled like this:

  $ gcc -std=gnu89 -Wall -o farmer farmer.c output.c -lrt -lX11

  $ gcc -std=gnu89 -Wall -o worker worker.c -lrt

Executed:
  ./farmer
  
No of workers and the size of the image can be changed in settings.h.
