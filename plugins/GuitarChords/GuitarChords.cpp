/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2018 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
THERE IS A BUG, if you change the chordType while still playing one some notes will not receive noteOFF
*/
#include "DistrhoPlugin.hpp"
#include "chords.hpp"
#include <stdio.h>
START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

class GuitarChords : public Plugin
{
public:
    GuitarChords()
        : Plugin(2, 0, 0) {} //one parameter, one program
        
         
protected:
   /* --------------------------------------------------------------------------------------------------------
    * Information */

   /**
      Get the plugin label.
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const override
    {
        return "GuitarChords";
    }

   /**
      Get an extensive comment/description about the plugin.
    */
    const char* getDescription() const override
    {
        return " A 'real' midi guitar chord generator. Different octaves will send different positions of the same chord and a second note played on top of the root will tell the chord type(Maj-Min...)";
    }

   /**
      Get the plugin author/maker.
    */
    const char* getMaker() const override
    {
        return "Haradai";
    }

   /**
      Get the plugin homepage.
    */
    const char* getHomePage() const override
    {
        return "";
    }

   /**
      Get the plugin license name (a single line of text).
      For commercial plugins this should return some short copyright information.
    */
    const char* getLicense() const override
    {
        return "ISC";
    }

   /**
      Get the plugin version, in hexadecimal.
    */
    uint32_t getVersion() const override
    {
        return d_version(1, 0, 0);
    }

   /**
      Get the plugin unique Id.
      This value is used by LADSPA, DSSI and VST plugin formats.
    */
    int64_t getUniqueId() const override
    {
        return d_cconst('d', 'M', 'T', 'r');
    }

   /* --------------------------------------------------------------------------------------------------------
    * Init and Internal data, unused in this plugin */

    void  initParameter(uint32_t index, Parameter& parameter) override {
        switch(index){
            case 0:
                parameter.hints = kParameterIsAutomable;
                parameter.ranges.min = 0.0;
                parameter.ranges.max = 1.0;
                parameter.ranges.def = 1.0;
                parameter.name = "Strum Speed";
                parameter.symbol = "STRUM";
                break;
            case 1:
                parameter.hints = kParameterIsAutomable|kParameterIsBoolean;
                parameter.ranges.min = 0.0;
                parameter.ranges.max = 1.0;
                parameter.ranges.def = 1.0;
                parameter.name = "ON/OFF";
                parameter.symbol = "ON/OFF";
                break;
        }
    }
    float getParameterValue(uint32_t index) const   override { 
        switch(index){
            case 0:
                return StrSpeed;
                break;
            case 1:
                return on_off;
                break;
        }
        return 0;
    }
    void  setParameterValue(uint32_t index, float value)  override {
        switch(index){
            case 0:
                StrSpeed = value;
                break;
            case 1:
                on_off = value;
                break;
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Audio/MIDI Processing */ 

   /**
      Run/process function for plugins with MIDI input.
      In this case we just pass-through all MIDI events.
    */
    void run(const float**, float**, uint32_t,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {   
        /* Debug prints (show midi inputs)
        for (int i = 0; i < midiEventCount; i++)
        {
            printf("%d , (%d) ",midiEvents[i].data[1],midiEvents[i].data[0]);
        }
        if(midiEventCount != 0)
        {
            printf("\n");
        }
        */
        

        //init function
        if (init == false)
        { 
            rootNote.data[1] = 127; //127 represents no note
            chordNote.data[1] = 127;
            addNote.data[1] = 127;
            for (int i = 0; i < 6; i++)
            {
                notesON[i].data[1] = 127;
                chord[i].data[1] = 127;
            }
            
            init = true;
        }

        //We gather input notes
        for (uint32_t i = 0; i < midiEventCount; i++)
        {
            //We look for the ON notes from the input feed
            if(midiEvents[i].data[0] >= 0x90 && midiEvents[i].data[0] <= 0x9F)
            {
                //we add new input note to input notes list
                int a = 0;
                while (notesON[a].data[1] != 127 && notesON[a].data[1] != midiEvents[i].data[1])
                {
                    a++;
                    if(a == 6) //so it doesn't crash if more than 6 note printf("%d , (%d) ",midiEvents[i].data[1],midiEvents[i].data[0]);s are inputed
                    {
                        notesON[0] = midiEvents[i];
                        a=5;
                        break;
                    }
                }
                notesON[a] = midiEvents[i];
            }

            //we look for the OFF notes from the input feed
            if((midiEvents[i].data[0] >= 0x80 && midiEvents[i].data[0] <= 0x8F) || midiEvents[i].data[2] == 0)
            {
                //we remove from the list input note
                int b = 0;
                while(notesON[b].data[1] != midiEvents[i].data[1])
                {
                    b++;
                    if(b == 6) //so it doesn't crash 
                    {
                        break;
                        b--;
                    }
                }
                notesON[b].data[1] = 127;
            }
        }
///////////////////////////////////////////////////////////////////////////////////////
/////////we check if rootNote, chordNote or addNote are off
///////////////////////////////////////////////////////////////////////////////////////

        //first we check for rootNote
        for (int i = 0; i < 6; i++)
        {
            if (notesON[i].data[1] == rootNote.data[1])
            {
                break;
            }
            
            //if we can't find rootNote on the pressed notes, we make rootNote = chordNote and later on we see if chordNote is pressed
            if(i == 5 && notesON[i].data[1] != rootNote.data[1])
            {
                rootNote = chordNote;
                chrdChng = true;
            }
        }

        //Now we look for chordNote
        for (int i = 0; i < 6; i++)
        {
            if (notesON[i].data[1] == chordNote.data[1])
            {
                //if it is pressed and rootNote was off in other words, it was assigned to = chordnote, chordnote will be off and the 
                // new rootNote is the previous chordNote
                if (rootNote.data[1] == chordNote.data[1] && rootNote.data[1] != 127)
                {
                    chordNote.data[1] = 127;
                    chrdChng = true;
                }
                break;
            }

            //if we can't find chordNote on the pressed notes, we make chordNote = addNote and later on we see if Note is pressed
            if(i == 5 && notesON[i].data[1] != chordNote.data[1])
            {
                
                if(chordNote.data[1] == rootNote.data[1])
                {
                    rootNote.data[1] = addNote.data[1];
                }
                chordNote.data[1] = addNote.data[1];
                chrdChng = true;
            }
        }

        //Now we look for addNote
        for (int i = 0; i < 6; i++)
        {
            if (notesON[i].data[1] == addNote.data[1])
            {
                if (rootNote.data[1] == addNote.data[1] && rootNote.data[1] != 127)
                {
                    chordNote.data[1] = 127;
                    addNote.data[1] = 127;
                    chrdChng = true;
                }
                else if (chordNote.data[1] == addNote.data[1] && chordNote.data[1] != 127)
                {
                    addNote.data[1] = 127;
                }
                
                break;
            }

            //if we can't find addNote on the pressed notes
            if(i == 5 && notesON[i].data[1] != chordNote.data[1])
            {  
                if(addNote.data[1] == rootNote.data[1])
                {
                    rootNote.data[1] = 127;
                }

                if(addNote.data[1] == chordNote.data[1])
                {
                    chordNote.data[1] = 127;
                }
                addNote.data[1] = 127;
                chrdChng = true;
            }
        }
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
///////////////////Now we set what notes are rootNote, chordNote and addNote
/////////////////////////////////////////////////////////////////////////////
        
        //Now we look for the two lowest notes of the input and compare them to rootNote and chordNote 
        for (int i = 0; i < 6; i++)
        {   
            //check for change as rootNote
            if(notesON[i].data[1] < rootNote.data[1])
            {
                chrdChng = true;
                rootNote = notesON[i];
            }
            
            if (notesON[i].data[1] > rootNote.data[1] && notesON[i].data[1] < chordNote.data[1])
            {
                chrdChng = true;
                chordNote = notesON[i];
            } 

            if (notesON[i].data[1] > chordNote.data[1] && notesON[i].data[1] < addNote.data[1])
            {
                chrdChng = true;
                addNote = notesON[i];
            } 
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////Now we do the actual generation of the chord and sending the midi messages////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if( on_off == true)
        {
            if (chrdChng == true)
            { 
                for (int i = 0; i < 6; i++) //chord off signal
                {
                    chord[i].data[0] = 0x80; 
                    chord[i].data[2] = 100; //would be cool to gather the rootNote and/or chordNote off velocity 
                    if (chord[i].data[1] != 127)
                    {
                        writeMidiEvent(chord[i]);
                        //printf("\n%d (%d) vel(%d) OFF",chord[i].data[1],chord[i].data[0],chord[i].data[2]);
                        chord[i].data[1] = 127;
                    }
                }
                
            
                if (rootNote.data[1] != 127)
                {
                    makeChord(chord,rootNote,chordNote,addNote); //generate chord
                    for (int i = 0; i < 6; i++) //send chord notes out
                    {
                        if(chord[i].data[1] != 127)
                        {
                                writeMidiEvent(chord[i]); 
                                //printf("\n%d (%d) vel(%d) ON",chord[i].data[1],chord[i].data[0],chord[i].data[2]);
                        } 
                    }
                }
                chrdChng = false;
            }
        }
        
        else{
            for (uint32_t i = 0; i < midiEventCount; i++)
            {
                writeMidiEvent(midiEvents[i]); //act as midi passthrough
            }
        }
         

        //debug
        /*
        printf("\nrootNote:%d (%d) , chordNote:%d (%d) , addNote:%d (%d) ,Dist: %d ,chord: %d (%d)",rootNote.data[1],rootNote.data[0] ,chordNote.data[1],chordNote.data[0],addNote.data[1],addNote.data[0],0,chord[0].data[1],chord[0].data[0]);
        
        for (int i = 1; i < 6; i++)
        {
            printf(",%d (%d)",chord[i].data[1],chord[i].data[0]);
        }
        printf("\n");
        */
        //printf("\n %d,%d,%d,%d,%d,%d",notesON[0].data[1],notesON[1].data[1],notesON[2].data[1],notesON[3].data[1],notesON[4].data[1],notesON[5].data[1]);
        
        
    }


private:

    MidiEvent chorda[6];  
    MidiEvent *chord = chorda;

    MidiEvent rootNote;
    MidiEvent chordNote;
    MidiEvent addNote; 
    MidiEvent notesON[6]; //really the user should only input two at a time

    
    
    //It seems I cannot do this, so I create an extra variable as a startup function
    //rootNote.data[0] = 0x80;
    bool init = false;
    bool chrdChng = false; //"chord change?"

    float StrSpeed = 0.0;
    bool on_off = true;
    
   /**
      Set our plugin class as non-copyable and add a leak detector just in case.
    */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarChords)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin()
{
    return new GuitarChords();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO