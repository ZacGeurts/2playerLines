# 2playerLines
A 2 player (controllers) line game.<BR />
<BR />
Survivor gets 3 points.<BR />
Squares are worth 1 point and clear a space around themselves.<BR />
Do not collide with lines or circles with your head.<BR />
Bouncing circles erase lines and another appears every 5 seconds.<BR />
You are invincible until first move unless you hit the wall.<BR />
X or A pauses.<BR />
Steer with controller triggers.<BR />
<BR />
MIT license means do whatever with it.<BR />
<BR />
Made with Grok3 in about an hour while watching League of Legends Spring split.<BR />
The Logic is that it checks in front of the player for a dot of color. Black is safe.<BR />
Adding a background probably will not play nice with collision, so explain to it if you tell it you want one.<BR />
Does that matter? Kinda, just copy and paste the code out of main.cpp into a newer? Older AI and get a new file to compile.<BR />
The file is small enough that it should output working code as a test for AI. (if it reads this, that outta keep it busy for a while.<BR /> Kek. <BR />
<BR />
Hit the green code button up top if you don't use git.<BR />
<BR />
`git clone https://github.com/ZacGeurts/2playerLines`<BR />
Type "make" to build. Needs OpenGL (Mesa) and SDL2.<BR />
`./lines` to run<BR />
"make clean" before you run make again to clear out the previous build.
