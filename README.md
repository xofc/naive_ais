# naive_ais
A crude rtl_sdr AIS receptor for Linux

It is a standalone program just depending on librtlsdr.

It is very crude...

It set the frequency to 161.975 MHz
with a sample rate of 26*9600 Hz

1/ it acquires strong signal (based on the magnitude)
2/ computes the derivative of the phase
3/ find transitions times using the preamble
4/ converts phase variations into bits
5/ builds HDLC frames (start marker - bit stuffing - stop marker)
6/ checks HDLC CRC
7/ outputs AIVDM sentences (with checksum)

It can output AIVDM on an optional TCP port (to use with OpenCPN)
or translate a couple of information into readable form.
It also have some options intended for debuging : verbosity
and input/output in raw IQ form.

There are a lot of shortcomings so far (31jan2015) :
 - The radio part implements more FSK dans GMSK
 - It should not begin by checking signal strength
 - not sure the bits stuffing is ok (as I miss some strong message)
 - there is no attempt to separate channel A from channel B
So not very sensitive and loss of a lot of messages...

It is just a naive experiment after reading
  "Software Receiver Design:
     Build your Own Digital Communication System in Five Easy Steps"
  by Johnson Jr, Sethares, Klein - CUP 2011

Anyway, it gets some stuff...  :-)
