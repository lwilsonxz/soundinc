#include <stdio.h>
#include <stdlib.h>
#include "wave.h"
#include <limits.h>

//Prototypes.
short getShort();
int getSamples( WaveHeader* header );
void writeShort(short sample);
int readInput(int argc, char** argv, short** left, short** right, WaveHeader* header);
void writeOutput( short* left, short* right, WaveHeader* header );
void reverse(short* left, short* right, WaveHeader* header);
void flip(short* left, short* right, WaveHeader* header);
void changeSpeed(short** left, short** right, double factor, WaveHeader* header);
void echo(short** left, short** right, double delay, double scale, WaveHeader* header);
void scaleVolume(short* left, short* right, double factor, WaveHeader* header);
void fadeIn(short* left, short* right, double seconds, WaveHeader* header );
void fadeOut(short* left, short* right, double seconds, WaveHeader* header );

int main(int argc, char** argv)
{
    int i = 0;
	int errorFlag = 0;
    short* left;
    short* right;
    int samples;
    WaveHeader header;

    readHeader(&header); //Reads .wav and brings us to where data chunk is.

	if ( header.ID[0] != 'R' || header.ID[1] != 'I' || header.ID[2] != 'F' || header.ID[3] != 'F' )
	{
		fprintf(stderr, "File is not a RIFF file\n");
		return 1;
	}

	if ( header.formatChunk.ID[0] != 'f' || header.formatChunk.ID[1] != 'm' || header.formatChunk.ID[2] != 't' )
	{
		fprintf(stderr, "Format chunk is corrupted\n");
		return 1;
	}
	else if ( header.formatChunk.size != 16 || header.formatChunk.compression != 1 )
	{
		fprintf(stderr, "Format chunk is corrupted\n");
		return 1;
	}

	if ( header.dataChunk.ID[0] != 'd' || header.dataChunk.ID[1] != 'a' || header.dataChunk.ID[2] != 't' || header.dataChunk.ID[3] != 'a' )
	{
		fprintf(stderr, "Format chunk is corrupted\n");
		return 1;
	}

	if ( header.formatChunk.channels != 2 )
		{
			fprintf(stderr, "File is not stereo\n");
			return 1;
		}

	if ( header.formatChunk.sampleRate != 44100 )
		{
			fprintf(stderr, "File does not use 44,100Hz sample rate\n");
			return 1;
		}

	if ( header.formatChunk.bitsPerSample != 16 )
		{
			fprintf(stderr, "File does not have 16-bit samples\n");
			return 1;
		}

    samples = getSamples( &header );

    left = (short*)malloc(sizeof(short) * samples);
    right = (short*)malloc(sizeof(short) * samples);

	if ( left == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}
	if ( right == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}

    //Loop to put data into left and right channels.
    for(i = 0; i < samples; ++i)
    {
            left[i] = getShort();
			/*if ( left[i] == -1 )
				return 0;*/ //this error check doesn't work
            right[i] = getShort();
			/*if ( right[i] == -1 )
				return 0;*/
    } //End for.
   
    errorFlag = readInput(argc, argv, &left, &right, &header);
	if ( errorFlag != 0 )
		return;
	writeOutput(left, right, &header);

    free(left);
    free(right);
} //End main.

//Reads 2 bytes and combines into short.
short getShort()
{
    short sample;
    char* pointer;

    pointer = (char*)&sample;

    pointer[0] = getchar();
	/*if ( pointer[0] == EOF )
	{
		fprintf(stderr, "File size does not match size in header\n");
		//return -1;
	}*/

    pointer[1] = getchar();
	/*if ( pointer[1] == EOF )
	{
		fprintf(stderr, "File size does not match size in header\n");
		//return -1;
	}*/

    return sample;
}//End getShort.

//Writes samples to stdout.
void writeShort(short sample)
{
    char* pointer = (char*)&sample;
 
    putchar(pointer[0]);
    putchar(pointer[1]);
} //End writeShort.

//Reads command line arguments.
int readInput(int argc, char** argv, short** left, short** right, WaveHeader* header)
{
	
	int samples = getSamples( header );
    int i = 0;
    char c;
    double d = 0; //factor, scale, seconds.
    double dd = 0; //factor, scale, seconds.

    for (i = 1 ; i < argc ; ++i)
    {
        c = argv[i][1];
       
        if (c == 'r')
            reverse(*left, *right, header);

        else if (c == 'f')
            flip(*left, *right, header);

        else if (c == 's')
        {
            d = atof(argv[i+1]);
			if ( d <= 0 )
			{
				fprintf(stderr, "A positive number must be supplied for the speed change\n");
				return 0;
			}
            changeSpeed(left, right, d, header);
            ++i;
        }

        else if (c == 'o')
        {
            d = atof(argv[i+1]);
			if ( d <= 0 )
			{
				fprintf(stderr, "A positive number must be supplied for the fade in and fade out time\n");
				return 0;
			}
            fadeOut(*left, *right, d, header);
            ++i;
        }

        else if (c == 'i')
        {
            d = atof(argv[i+1]);
			if ( d <= 0 )
			{
				fprintf(stderr, "A positive number must be supplied for the fade in and fade out time\n");
				return 0;
			}
            fadeIn(*left, *right, d, header);
            ++i;
        }

        else if (c == 'v')
        {
            d = atof(argv[i+1]);
			if ( d <= 0 )
			{
				fprintf(stderr, "A positive number must be supplied for the volume scale\n");
				return 0;
			}
            scaleVolume(*left, *right, d, header);
            ++i;
        }

        else if (c == 'e')
        {
            d = atof(argv[i+1]);
            dd = atof(argv[i+2]);
			if ( d <= 0 || dd <= 0 )
			{
				fprintf(stderr, "Positive numbers must be supplied for the echo delay and scale parameters\n");
				return 0;
			}
            echo(left, right, d, dd, header);
            i += 2;
        }

       else
		{
       		fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
			return 1;
		}
    } //End for.
	return 0;
} //End readInput.

//Writes output .wav file.
void writeOutput(short* left, short* right, WaveHeader* header)
{
	int i = 0;
	int samples = getSamples( header );

	writeHeader(header); //Puts RIFF back on.

	for (i = 0 ; i < samples ; ++i)
	{
		writeShort(left[i]); //segmentation fault seems to occur here
		writeShort(right[i]);
	}
} //End writeOutput.

//Reverses the audio.
void reverse(short* left, short* right, WaveHeader* header)
{
	int samples = getSamples( header );
	int i = 0;
	int j = samples - 1;
	short tempLeft;
	short tempRight;

	while(i < (samples / 2))
	{
		tempLeft = left[i]; //Stores first element.
		tempRight = right[i];

		left[i] = left[j]; //Replaces first element with last.
		right[i] = right[j];

		left[j] = tempLeft; //Replaces last element with stored first.
		right[j] = tempRight;

		++i;
		--j;
	} //End while.
} //End reverse.

//Flips left and right stereo channels.
void flip(short* left, short* right, WaveHeader* header)
{
	int samples = getSamples( header );
	int i = 0;
	short temp;

	for(i = 0; i < samples; ++i)
	{
		temp = left[i];
		left[i] = right[i];
		right[i] = temp;
	} //End for.
} //End flip.

//Change audio speed by given factor.
void changeSpeed(short** left, short** right, double factor, WaveHeader* header)
{
	int i = 0;
	int samples = getSamples( header );
	int newSamples = (int)(samples / factor);
	short* newLeft = (short*)malloc(sizeof(short) * newSamples);
	short* newRight = (short*)malloc(sizeof(short) * newSamples);

	if ( newLeft == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}
	if ( newRight == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}

	for(i = 0; i < newSamples; ++i)
	{
			//fprintf(stderr, "In the Loop!\n");
			newLeft[i] = (*left)[(int)(i * factor)];
			//fprintf(stderr, "Loop middle\n");
			newRight[i] = (*right)[(int)(i * factor)];
			//	fprintf(stderr, "Loop end!\n");
	} //End for.

	header->size = 36 + newSamples * 4;

	header->dataChunk.size = newSamples * 4;

	free(*left);
	free(*right);
	*left = newLeft;
	*right = newRight;
} //End changeSpeed.

void fadeOut(short* left, short* right, double seconds, WaveHeader* header)
{
	int i = 0;
	int j = 0;
	int samples = getSamples( header );
	double fadeSize = seconds * 44100;
	int fadeIndex = (int)(samples - fadeSize);

	for ( i = 0 ; i < samples ; ++i )
	{
		if ( i >= fadeIndex )
		{
			left[i] *= (1 - j / fadeSize) * (1 - j / fadeSize);
			right[i] *= (1 - j / fadeSize) * (1 - j / fadeSize);
			++j;
		}
	}
} //End fadeOut.

void fadeIn(short* left, short* right, double seconds, WaveHeader* header)
{
	int i = 0;
	int j = 0;
	int samples = getSamples( header );
	double fadeSize = seconds * 44100;
	int fadeIndex = (int)(samples - fadeSize);

	for ( i = 0 ; i < samples ; ++i )
	{
		if ( i <= fadeSize )
		{
			left[i] *= (j / fadeSize) * (j / fadeSize);
			right[i] = (j / fadeSize) * (j / fadeSize);
			++j;
		}
	}
} //End fadeIn.

void scaleVolume(short* left, short* right, double factor, WaveHeader* header)
{
	int i = 0;
	int samples = getSamples( header );
	
	for ( i = 0 ; i < samples ; ++i )
	{
		if ( (int)(left[i] * factor) < SHRT_MAX && (int)(left[i] * factor) > SHRT_MIN )//if stuff is normal, just stick it in there
			left[i] = (int)(left[i] * factor);
		else if ( (int)(left[i] * factor) >= SHRT_MAX )
			left[i] = SHRT_MAX;
		else
			left[i] = SHRT_MIN;
		if ( (int)(right[i] * factor) < SHRT_MAX && (int)(right[i] * factor) > SHRT_MIN )//ibid.
			right[i] = (int)(right[i] * factor);
		else if ( (int)(right[i] * factor) >= SHRT_MAX )
			right[i] = SHRT_MAX;
		else
			right[i] = SHRT_MIN;
	}
} //End scaleVolume.

void echo(short** left, short** right, double delay, double scale, WaveHeader* header)
{
	int i = 0;
	int j = 0;
	int samples = getSamples( header );
	int newSamples = samples + (int)(delay * header->formatChunk.sampleRate);

	short* newLeft = (short*)malloc(sizeof(short) * newSamples);
	short* newRight = (short*)malloc(sizeof(short) * newSamples);

	if ( newLeft == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}
	if ( newRight == NULL )
	{
		fprintf(stderr, "Program out of memory\n");
		return;
	}

	for ( i = 0 ; i < newSamples ; ++i )
	{
		if ( i > (int)(delay * header->formatChunk.sampleRate) && i < samples )
		{
			newLeft[i] = (*left)[i] + (int)((*left)[j] * scale);
			newLeft[i] = (*right)[i] +(int)((*left)[j] * scale);
			++j;
		} //echo and overlap

		else if ( i > (int)(delay * header->formatChunk.sampleRate) )
		{
			newLeft[i] = (int)((*left)[j] * scale);
			newLeft[i] = (int)((*left)[j] * scale);
			++j;
		}

		else
		{
			newLeft[i] = (*left)[i];
			newRight[i] = (*right)[i];

		} //don't echo yet
	}

	header->size = 36 + newSamples * 4;

	header->dataChunk.size = newSamples * 4;

	free(left[0]);
	free(right[0]);
	*left = newLeft;
	*right = newRight;
} //End echo.

int getSamples( WaveHeader* header )
{
	return header->dataChunk.size / (header->formatChunk.channels * header->formatChunk.bitsPerSample / 8);
}
