obsolete code here!

check libraries for reading and writing:

https://github.com/tomaskovacik/VAGFISReader
https://github.com/tomaskovacik/VAGFISWriter


section dedicated to 3wire comunication betwen radio and cluster (FIS) on vw group vehicles in years to 1999 maybe laters (I have 1999 audi), more: http://kovo-blog.blogspot.sk/2013/11/audi-fis-3-line-protocol.html

- `dumps` contains logic analyzer dumps
- `FIS_protocol_emulator` writes to a real FIS cluster.  It reads lines over a
  serial port (9600 bps, lines terminated by `\n`) and writes the lines to a
  real FIS cluster over 3 line bus.
- `read_cluster` emulates an FIS cluster.  It listens on the 3 line bus like
  a real FIS cluster would and writes what it receives to a 2x16 character LCD.

all of this is tested in setup on bench: writing to display:

https://www.youtube.com/watch?v=8igXlCWadeE

reading display:
https://www.youtube.com/watch?v=ANw7_7F3OmM


