#include "DistrhoPlugin.hpp"
#include "chords.hpp"
#include <stdio.h>

/* We define these here so they don't have to be included every time we call a chord function*/

MidiEvent *CHORD;
MidiEvent ROOTnOTE;
int OCT;

//We define the variables for the functions copied from the le strum project
enum type{
	CHORD_NONE 	= 0,
	CHORD_MAJ 	= 1,
	CHORD_MIN	= 2,
	CHORD_DOM7	= 3,
	CHORD_MAJ7  = 4,
	CHORD_MIN7  = 5,
	CHORD_AUG   = 6,
	CHORD_DIM	= 7
};

// CHORD ROOT NOTES
enum root{
	ROOT_C = 0,
	ROOT_CSHARP = 1,
	ROOT_D = 2, 
	ROOT_DSHARP = 3,
	ROOT_E = 4,
	ROOT_F = 5,
	ROOT_FSHARP = 6,
	ROOT_G = 7,
	ROOT_GSHARP = 8,
	ROOT_A = 9,
	ROOT_ASHARP = 10,
	ROOT_B = 11
};

// CHORD EXTENSIONS
enum add{
	ADD_NONE,
	SUS_4 = 1,
	ADD_6 = 2,
	ADD_9 = 3
};

// CONTROLLING FLAGS
enum mode{
	OPT_GUITAR				= 0x0020, // use guitar voicing
	OPT_GUITAR2 			= 0x0040, // use octave shifted guitar chord map on strings 10-16
	OPT_GUITARBASSNOTES		= 0x0080, // enable bottom guitar strings (that are usually damped) but can provide alternating bass notes
	OPT_SUSTAIN				= 0x0100, // do not kill all strings when chord button is released
	OPT_SUSTAINCOMMON		= 0x0200, // when switching to a new chord, allow common notes to sustain (do not retrig) on strings
	OPT_ADDNOTES			= 0x1000, // enable adding of sus4, add6, add9 to chord
};

mode options = OPT_GUITARBASSNOTES;

//Functions 
void guitarCShape (uint8_t  ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
		chord[0].data[1] = 43 + ofs;
    }
	chord[1].data[1] = 48 + ofs;
	chord[2].data[1] = 52 + ofs + (extension == SUS_4);
	chord[3].data[1] = 55 + ofs + 2 * (extension == ADD_6);
	chord[4].data[1] = 60 + ofs + 2 * (extension == ADD_9);
	chord[5].data[1] = 64 + ofs + (extension == SUS_4);
}

void guitarC7Shape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 43 + ofs;
    }
	chord[1].data[1] = 48 + ofs;
	chord[2].data[1] = 52 + ofs + (extension == SUS_4);
	chord[3].data[1] = 58 + ofs - (extension == ADD_6);
	chord[4].data[1] = 60 + ofs + 2 * (extension == ADD_9);
	chord[5].data[1] = 64 + ofs + (extension == SUS_4);
}
void guitarAShape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[1].data[1] = 45 + ofs;
	chord[2].data[1] = 52 + ofs + 2 * (extension == ADD_6);
	chord[3].data[1] = 57 + ofs + 2 * (extension == ADD_9);;
	chord[4].data[1] = 61 + ofs + (extension == SUS_4);
	chord[5].data[1] = 64 + ofs;
}
void guitarAmShape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[1].data[1] = 45 + ofs;
	chord[2].data[1] = 52 + ofs + 2 * (extension == ADD_6);
	chord[3].data[1] = 57 + ofs + 2 * (extension == ADD_9);;
	chord[4].data[1] = 60 + ofs  + 2 * (extension == SUS_4);
	chord[5].data[1] = 64 + ofs;
}
void guitarA7Shape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[1].data[1] = 45 + ofs;
	chord[2].data[1] = 52 + ofs + 2 * (extension == ADD_6);
	chord[3].data[1] = 55 + ofs + 4 * (extension == ADD_9);;
	chord[4].data[1] = 61 + ofs + (extension == SUS_4);
	chord[5].data[1] = 64 + ofs;
}
void guitarDShape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[2].data[1] = 50 + ofs;
	chord[3].data[1] = 57 + ofs + 2 * (extension == ADD_6);
	chord[4].data[1] = 62 + ofs;
	chord[5].data[1] = 66 + ofs  + (extension == SUS_4) - 2*(extension == ADD_9);
}
void guitarDmShape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[2].data[1] = 50 + ofs;
	chord[3].data[1] = 57 + ofs + 2 * (extension == ADD_6);
	chord[4].data[1] = 62 + ofs;
	chord[5].data[1] = 65 + ofs  + 2 * (extension == SUS_4) - (extension == ADD_9);
}
void guitarD7Shape(int ofs, add extension, MidiEvent *chord)
{
	if(options & OPT_GUITARBASSNOTES)
    {
        chord[0].data[1] = 40 + ofs;
    }	
	chord[2].data[1] = 50 + ofs;
	chord[3].data[1] = 57 + ofs + 2 * (extension == ADD_6);
	chord[4].data[1] = 60 + ofs;
	chord[5].data[1] = 66 + ofs  + (extension == SUS_4)- 2*(extension == ADD_9);
}
void guitarEShape(int ofs, add extension, MidiEvent *chord)
{
	chord[0].data[1] = 40 + ofs;
	chord[1].data[1] = 47 + ofs;
	chord[2].data[1] = 52 + ofs + 2 * (extension == ADD_9);
	chord[3].data[1] = 56 + ofs  + (extension == SUS_4);
	chord[4].data[1] = 59 + ofs + 2 * (extension == ADD_6);
	chord[5].data[1] = 64 + ofs;
}
void guitarEmShape(int ofs, add extension, MidiEvent *chord)
{
	chord[0].data[1] = 40 + ofs;
	chord[1].data[1] = 47 + ofs;
	chord[2].data[1] = 52 + ofs + 2 * (extension == ADD_9);
	chord[3].data[1] = 55 + ofs  + 2 * (extension == SUS_4);
	chord[4].data[1] = 59 + ofs + 2 * (extension == ADD_6);
	chord[5].data[1] = 64 + ofs;
}
void guitarE7Shape(int ofs, add extension, MidiEvent *chord)
{
	chord[0].data[1] = 40 + ofs;
	chord[1].data[1] = 47 + ofs;
	chord[2].data[1] = 50 + ofs + 4 * (extension == ADD_9);
	chord[3].data[1] = 56 + ofs  + (extension == SUS_4);
	chord[4].data[1] = 59 + ofs + 2 * (extension == ADD_6);
	chord[5].data[1] = 64 + ofs;
}
void guitarGShape(int ofs, add extension, MidiEvent *chord)
{
	chord[0].data[1] = 43 + ofs;
	chord[1].data[1] = 47 + ofs  + (extension == SUS_4);
	chord[2].data[1] = 50 + ofs + 2 * (extension == ADD_6);
	chord[3].data[1] = 55 + ofs  + 2*(extension == ADD_9);
	chord[4].data[1] = 59 + ofs + (extension == SUS_4);
	chord[5].data[1] = 67 + ofs;
}

////////////////////////////////////////////////////////////
//
// GUITAR CHORD MAPPING
//
////////////////////////////////////////////////////////////
void makeChord(MidiEvent *chord , MidiEvent rootNote , MidiEvent chordNote ,MidiEvent addNote)
{	
	root RtNote = (root)(rootNote.data[1] - ((int)(rootNote.data[1]/12)*12));
	type chdNote = (type)(chordNote.data[1] - rootNote.data[1]);
	add  extNote = (add)(addNote.data[1] - chordNote.data[1]); 
	//printf("\n%d", (int)RtNote);

	switch(chdNote)
	{
		case CHORD_MAJ:
			switch(RtNote)
			{
			case ROOT_C:		guitarCShape(0, extNote, chord);	break;
			case ROOT_CSHARP:  	guitarAShape(4, extNote, chord);	break;
			case ROOT_D:		guitarDShape(0, extNote, chord);	break;
			case ROOT_DSHARP:	guitarAShape(6, extNote, chord);	break;
			case ROOT_E:		guitarEShape(0, extNote, chord);	break;
			case ROOT_F:		guitarEShape(1, extNote, chord);	break;
			case ROOT_FSHARP:	guitarEShape(2, extNote, chord);	break;
			case ROOT_G:		guitarGShape(0, extNote, chord);	break;
			case ROOT_GSHARP:	guitarEShape(4, extNote, chord);	break;
			case ROOT_A:		guitarAShape(0, extNote, chord);	break;
			case ROOT_ASHARP:   guitarAShape(1, extNote, chord);	break;
			case ROOT_B:   		guitarAShape(2, extNote, chord);	break;
			}
			break;
		case CHORD_MIN:
			switch(RtNote)
			{
			case ROOT_C:		guitarAmShape(3, extNote, chord);	break;
			case ROOT_CSHARP:  	guitarAmShape(4, extNote, chord);	break;
			case ROOT_D:		guitarDmShape(0, extNote, chord);	break;
			case ROOT_DSHARP:	guitarAmShape(6, extNote, chord);	break;
			case ROOT_E:		guitarEmShape(0, extNote, chord);	break;
			case ROOT_F:		guitarEmShape(1, extNote, chord);	break;
			case ROOT_FSHARP:	guitarEmShape(2, extNote, chord);	break;
			case ROOT_G:		guitarEmShape(3, extNote, chord);	break;
			case ROOT_GSHARP:	guitarEmShape(4, extNote, chord);	break;
			case ROOT_A:		guitarAmShape(0, extNote, chord);	break;
			case ROOT_ASHARP:   guitarAmShape(1, extNote, chord);	break;
			case ROOT_B:   		guitarAmShape(2, extNote, chord);	break;
			}
			break;
		case CHORD_DOM7:
			switch(RtNote)
			{
			case ROOT_C:		guitarC7Shape(0, extNote, chord);	break;
			case ROOT_CSHARP:  	guitarA7Shape(4, extNote, chord);	break;
			case ROOT_D:		guitarD7Shape(0, extNote, chord);	break;
			case ROOT_DSHARP:	guitarA7Shape(6, extNote, chord);	break;
			case ROOT_E:		guitarE7Shape(0, extNote, chord);	break;
			case ROOT_F:		guitarE7Shape(1, extNote, chord);	break;
			case ROOT_FSHARP:	guitarE7Shape(2, extNote, chord);	break;
			case ROOT_G:		guitarE7Shape(3, extNote, chord);	break;
			case ROOT_GSHARP:	guitarE7Shape(4, extNote, chord);	break;
			case ROOT_A:		guitarA7Shape(0, extNote, chord);	break;
			case ROOT_ASHARP:   guitarA7Shape(1, extNote, chord);	break;
			case ROOT_B:   		guitarA7Shape(2, extNote, chord);	break;
			}
			break;
		
		default:
			break;
	}
    for (int i = 0; i < 6; i++)
    {
        chord[i].data[0] = 0x90;
        chord[i].data[2] = rootNote.data[2];
        chord[i].frame = 0;
        chord[i].size = 3;
    }
}

