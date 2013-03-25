llaudio
-------

This is a piece of work I did back in 2010, basically figuring out what the raw 
user/kernel audio interface looked like.  

The idea was to produce somethign which would allow audio on puredarwin


Some info at the time:

OSvKernDSPLib.kext:
We don't have the source but the functions that we need are all well documented 
in the ADC and simple (each perform a single mathematical operation).  It would 
be easy for someone to implement these and patch them directly into whatever 
needs them or, better still, create a kext which exports them called 
OSvKernDSPLib.kext.

Usermode access:
If you use shark to do a trace (on OSX) of a program recording audio you'll find 
LOTS of references to HP_ funcs, in particular HP_IOThread.  Now if you search 
over a full XCode install you'll find you actually have that code.

Turns out that a huge amount of the usermode Coreaudio code is shipped with 
XCode to allow the development/testing of CoreAudioPlugins; go look in the 
folders called HPBase, PublicUtility etc.  These classes etc call down to the 
lower level audio interface functions, header files: AudioHardware.h etc (we 
don't have the source) and these then communicate with the audio drivers 
IOUserClient in the kernel.

So, where does that leave us.  Thankfully by trawling over the HP_ code you can 
find out whats important for the user code (the key thing it realtime priority), 
and by trawling through the kernel source (IOKit) you can find out what 
interface the code in AudioHardware.h uses 
(http://lists.apple.com/archives/Darwin-dev/2009/Aug/msg00000.html).

What I posted back in August on the Darwin-dev list is basically correct and 
the answers to my questions are:

Q. Is there a mechanism to tell me when its safe to use this data, or is it, as
 I suspect, volatile and you use it "as it".

A. Sort of.  You basically need a realtime usermode thread which polls the 
kernel for the correct data.  The generic, kernel, audio stuff then gives you a 
copy of the samples your expecting.

Q. My key question is how do I get intelligible audio from the driver? I've 
tried putting a sleep in, but to no avail.

A. IIRC, I did do this over 6 months ago; it all starts working once you get 
your io thread working correctly.

To help illustrate all this I thought I'd attach my test program.  I know its a 
mess, but it does (or did last time I used it) read audio directly from the 
kernel on OSX and save it into a file.  What I did was pull out all the key 
things from the HP_IOThread func, combine that which what I'd learnt about the 
raw IOKIt interface and put it all together.  You should be able to load the 
output audio into something like Audacity and then play it.  Simples, see :)


License
------- 

Created by R J Cooper on 05/06/2012. This file: Copyright (c)
2012 Mountainstorm API: Copyright (c) 2008 Apple Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
