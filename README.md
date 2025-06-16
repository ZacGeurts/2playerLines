<!--- This files is to be viewed at https://github.com/ZacGeurts/2PlayerLines --->
# 2playerLines
A 2 player (controllers) line game.<BR />
![Screenshot](Screenshot.png)<BR />
The file is small enough that it should output working code as a test for AI.
<BR />

The Logic is that it checks in front of the player for a dot of color. Black is safe.<BR />
Adding a background probably will not play nice with collision depending on collision check order.
<BR />
MIT license means do whatever with it.<BR />
Get rich. whatever.<BR />
<BR />
The unaltered main.cpp version will be hosted here.<BR />
Please, do not redistribute main.cpp for profit, unaltered.<BR>
Have AI change main.cpp and put your name on it.<BR />
main.cpp is intended to run at some point on a rediculous number of systems that have some access to OpenGL and SDL.<BR />
I have a sideproject that is creating toolchains for the platforms.<BR />
<BR />
What does any of this mean?:<BR />
There is one small main.cpp that AI can read in entirety and write include files or other programs from that run on various platforms.<BR />
<BR />
If you have a tiny main.cpp that plays 2PlayerLines on your Dreamcast, you can do anything with main.cpp that uses sdl and opengl, like a platformer.<BR />
Even a new 4 player game if you have the controllers and friends.<BR />
main.cpp is your foot in the door.<BR />
<BR />
Planned platforms:
aarch64<BR />
armv6<BR />
armv7<BR />
armv8<BR />
dreamcast<BR />
nintendo_3ds<BR />
nintendo_switch<BR />
nintendo_wii<BR />
nintendo_wii_u<BR />
ouya<BR />
ps3<BR />
ps4<BR />
psp<BR />
ps_vita<BR />
steam_deck<BR />
steam_link<BR />
x86<BR />
x86_64<BR />
<BR />
Controls and about main.cpp
Survivor gets 3 points.<BR />
Squares are worth 1 point and clear a space around themselves.<BR />
Do not collide with lines or circles with your head.<BR />
Bouncing circles erase lines and another appears every 5 seconds.<BR />
You are invincible until first move unless you hit the wall.<BR />
X or A pauses.<BR />
Steer with controller triggers.<BR />
<BR />
To download source, hit the green code button up top if you don't use git.<BR />
<BR />
`git clone https://github.com/ZacGeurts/2playerLines`<BR />
`cd 2playerLines`
Type `make` to build. Needs OpenGL (Mesa) and SDL2.<BR />
`./lines` to run<BR />
Save a backup of your working builds if you cannot afford to lose them.<BR />
`make clean` before you run make again to clear out the previous build.<BR />
If you destroy your main.cpp, you can come download this one again.<BR />
Modifiing files other than main.cpp is not a beginner project.
