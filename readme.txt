Although C was not one of the main requested languages, I thought it might be fun to share some of my early work!

The goal of the project was to allow users to edit an audio file using C code. Supports wavs and seven commands:

Task		Command		Description
Reverse			-r		Reverse the audio
Change speed	-s		Change audio speed by a specific factor
Flip channels	-f		Flip the left and the right stereo channels
Fade out		-o		Fade out from full volume to zero at the end of the file over a specified time
Fade in			-i		Fade in from zero to full volume at the beginning of the file over a specified time
Volume			-v		Scale the volume of the file by a specified amount
Echo			-e		Add an echo to the sound with a specified delay and a specific scale factor

Command line arguments should look like so, as an example: 

./wave -r -f -v .75 -o 1 -e .1 .25 < input.wav > output.wav

This command takes the file input.wav as input, reverses it, flips its left and right channels, scales the volume to 75%, fades out its end over 1 second, and adds an echo with a delay of .1 seconds and a scale factor of .25. Finally, it sends the modified file to stdout where it is redirected into the file output.wav.

This was one of my earlier projects, but I thought it might be cool to include something I did in C. I have included in this folder a wav file, for convenience.