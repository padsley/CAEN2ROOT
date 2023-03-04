//header files to load useful stuff - TFile for output TTree, TString to handle filenames etc, iostream for input/output operations, sstream for something, maybe working with text again?
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <sstream>

//definitions of the two different ways of running the code, either with 'spectra' (meaning listmode = channel, time, energy) or waveforms (meaning the full sample wavefunction is stored)
void ProcessWaveforms(TString inFile, TString outFile);
void ProcessSpectra(TString inFile, TString outFile);

int EventWindow = 4;//how far apart different events can be to be combined into one full event
double TimeWindow = 100; //time window for making coincidence events (hopefully!)

//to run the code, CAEN2ROOT(input file name, output file name, flag for if it's listmode or scope samples
void CAEN2ROOT(TString inFile, TString outFile, bool waveforms=false)
{
    if(waveforms)
        ProcessWaveforms(inFile, outFile);
    else
        ProcessSpectra(inFile, outFile);
}

void ProcessWaveforms(TString inFile, TString outFile)
{
  //open the input file
    ifstream input;
    input.open(inFile.Data());

    //make an output file - overwrite the old one
    TFile *fout = new TFile(outFile.Data(),"RECREATE");

    //make an output TTree with times, channels, samples as the branches
    TTree *trout = new TTree("waveforms","waveforms");

    //make the data structures to put into in and...
    std::vector<long> vTimestamp, vChannel, vSamples;

    //... link those data structures to the branches
    trout->Branch("channel",&vChannel);
    trout->Branch("timestamp",&vTimestamp);
    trout->Branch("samples",&vSamples);

    //declare a variable for the channel for each event - set to -1 because this CAN'T EVER HAPPEN so it's a good check if something fucked up
    int channel = -1;
    
    int counter = 0;//only here to I can try reading a few events at a time
    
    TGraph *g = new TGraph();//for checking/plotting the shape
    
    if(input.is_open())
    {
//         while()//check for the end of the files? how many samples?
      //make something to read the line into
        std::vector<string> entry;
        //make somethings to break the line up into
        std::string line, word;
        
        while(getline(input, line) && counter<1)//this means only read one event because counter<1 - should fix this at some point... getline(input,line) reads a line from input into the variable 'line'
        {
	  std::cout << "New Entry" << std::endl;//printing stuff so I know things are happening

	  //CLEAR THE VECTORS TO AVOID WRITING THE SAME EVENTS TO DISK LOADS OF TIME AND EXPLODING THE COMPUTER POW POW POW
            vTimestamp.clear();
            vChannel.clear();
            vSamples.clear();            

	    //read line into str because ?????
            stringstream str(line);

	    //break str up into words and store them
            while(getline(str, word, ';'))
            {
                entry.push_back(word);
                
                std::cout << "word = " << word << std::endl;
            }
            //the timestamp is the first word (which is 0 because C-numbering)
            vTimestamp.push_back(stol(entry.at(0)));
	    //the channel is the second word (at position 1)
            vChannel.push_back(stol(entry.at(1)));
//             vSamples.push_back(entry.at(2));

	    //work out if the signal is positive or negative going (somehow??)
            bool NegativeSignal = false;
	    //stol is string to long so it's converting a text string to a number (I believe)
            if(stol(entry.at(4))<0)NegativeSignal = true;

	    //I'm not entirely clear why I did this
            if(NegativeSignal)vSamples.push_back(-1*stol(entry.at(4)));
            else vSamples.push_back(stol(entry.at(4)));

	    //loop over the number of samples which is an integer at position 3 in the output line and save the waveform values to the nice vector setup for this porpoise
            for(int i=1;i<stoi(entry.at(2));i++)
            {
                vSamples.push_back(stol(entry.at(i+4)));
            }

	    //save the waveform information into a TGraph that can be plotted if needed
            for(unsigned int i=0;i<vSamples.size()-2;i++)
            {
                std::cout << vSamples.at(i) << std::endl;
                g->SetPoint(i,i,vSamples.at(i));
            }

	    //iterate the counter for checking events
            counter++;
	    //fill the output TTree just saving each waveform separately
            trout->Fill();
        }
    }//everything up to this point is just reading in the waveforms but now we need to look at making proper energy values from them etc *le sigh*
    
    
    //draw the TGraph
    g->Draw("AP");

    //write the output TTree
    trout->Write();
    fout->Close();//close the output
    input.close();//close the input
    
    printf("Finished waveform conversion :)\n");//wooooo done
}

void ProcessSpectra(TString inFile, TString outFile)
{
    std::cout << "Processing listmode data" << std::endl;
    
    ifstream input;
    input.open(inFile.Data());
    
    TFile *fout = new TFile(outFile.Data(),"RECREATE");
    
    TTree *trout = new TTree("listmode","listmode");
    
    std::vector<long> vChannel, vTimestamp, vADCValue;
    
    trout->Branch("channel",&vChannel);
    trout->Branch("timestamp",&vTimestamp);
    trout->Branch("adc",&vADCValue);
    
    if(input.is_open())
    {
        vChannel.clear();
        vTimestamp.clear();
        vADCValue.clear();
        
        std::vector<string> entry;
        
        std::string line, word;
        
        while(getline(input, line))
        {
            entry.clear();
            vChannel.clear();
            vTimestamp.clear();
            vADCValue.clear();
            
            stringstream str(line);
                                 
            while(getline(str, word, ';'))
            {
                entry.push_back(word);
                
//                 std::cout << "word = " << word << std::endl;
            }
            
//             std::cout << "entry.size() = " << entry.size() << std::endl;
            
            vChannel.push_back(stol(entry[0]));
//             std::cout << "vChannel.size() = " << vChannel.size() << std::endl;
            
            vTimestamp.push_back(stol(entry[1]));
//             std::cout << "vTimestamp.size() = " << vTimestamp.size() << std::endl;
            
            vADCValue.push_back(stol(entry[2]));
//             std::cout << "vADCValue.size() = " << vADCValue.size() << std::endl;
            
            
            trout->Fill();
        }
//                 std::cout << channel << "\t" << timestamp << "\t" << adc << std::endl;
    }
    else
        std::cout << "Input file " << inFile.Data() << " doesn't seem to exist!" << std::endl;
    
    trout->Write();
    
    TTree *trent = new TTree("events","events");
    int EventLength;
    std::vector<long> EventTimes, EventADCs, EventChannels;
    
    trent->Branch("EventLength",&EventLength);
    trent->Branch("EventTimes",&EventTimes);
    trent->Branch("EventADCs",&EventADCs);
    trent->Branch("EventChannels",&EventChannels);
    
    std::vector<long> *ReadChannel = 0, *ReadTime = 0, *ReadADC = 0;
    TBranch *bReadChannel, *bReadTime, *bReadADC;
    
    TTree *ReadTrout = (TTree*)fout->Get("listmode");
    
    ReadTrout->SetBranchAddress("channel",&ReadChannel,&bReadChannel);
    ReadTrout->SetBranchAddress("timestamp",&ReadTime,&bReadTime);
    ReadTrout->SetBranchAddress("adc",&ReadADC,&bReadADC);
    
    long nentries = ReadTrout->GetEntries();
    for(long i=0; i<nentries; i++)
    {
//         ReadTrout->Show(i);
        ReadTrout->GetEntry(i);
        
        EventLength = 0;
        EventTimes.clear();
        EventADCs.clear();
        EventChannels.clear();
        
//         int VectorSize = ReadChannel->size();
//         std::cout << "VectorSize = " << VectorSize << std::endl;
        
//         std::cout << i << "\t" << ReadChannel->at(0) << std::endl;//just for testing
        if(ReadChannel->size()>1)std::cout << "For event " << i << " there are multiple hits in one channel and I did not account for that!" << std::endl;
        
        long TimeFirstEvent = ReadTime->at(0);
        
        long TimeLastEvent = TimeFirstEvent;
        
        long LastEventNumber = i;
        
        EventLength++;
        EventChannels.push_back(ReadChannel->at(0));
        EventTimes.push_back(ReadTime->at(0));
        EventADCs.push_back(ReadADC->at(0));
        
        for(long j=i+1; j<i+EventWindow; j++)//only look a few results ahead
        {
            ReadTrout->GetEntry(j);
            
//             std::cout << ReadTime->at(0) - TimeFirstEvent << std::endl;
            
            if(abs(ReadTime->at(0) - TimeFirstEvent)<TimeWindow)
            {
                EventLength++;
                EventChannels.push_back(ReadChannel->at(0));
                EventTimes.push_back(ReadTime->at(0));
                EventADCs.push_back(ReadADC->at(0));
                
                LastEventNumber = j;
            }
        }
        
        i = LastEventNumber;
        
//         std::cout << "la la la" << std::endl;//this is supposed to be useful??
        
        trent->Fill();
        
    }
    
    trent->Write();
    fout->Close();
    
    input.close();
    
}
