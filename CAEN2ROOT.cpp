#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <sstream>

void ProcessWaveforms(TString inFile, TString outFile);
void ProcessSpectra(TString inFile, TString outFile);

int EventWindow = 4;
double TimeWindow = 100; //time window for making coincidence events (hopefully!)

void CAEN2ROOT(TString inFile, TString outFile, bool waveforms=false)
{
    if(waveforms)
        ProcessWaveforms(inFile, outFile);
    else
        ProcessSpectra(inFile, outFile);
}

void ProcessWaveforms(TString inFile, TString outFile)
{
    ifstream input;
    input.open(inFile.Data());
    
    TFile *fout = new TFile(outFile.Data(),"RECREATE");
    
    TTree *trout = new TTree("waveforms","waveforms");
    
    std::vector<long> vTimestamp, vChannel, vSamples;
    
    trout->Branch("channel",&vChannel);
    trout->Branch("timestamp",&vTimestamp);
    trout->Branch("samples",&vSamples);
    
    int channel = -1;
    
    int counter = 0;//only here to I can try reading a few events at a time
    
    TGraph *g = new TGraph();//for checking the shape
    
    if(input.is_open())
    {
//         while()//check for the end of the files? how many samples?
        std::vector<string> entry;
        
        std::string line, word;
        
        while(getline(input, line) && counter<1)
        {
            std::cout << "New Entry" << std::endl;
            
            vTimestamp.clear();
            vChannel.clear();
            vSamples.clear();            
            
            stringstream str(line);
            
            while(getline(str, word, ';'))
            {
                entry.push_back(word);
                
                std::cout << "word = " << word << std::endl;
            }
            
            vTimestamp.push_back(stol(entry.at(0)));
            vChannel.push_back(stol(entry.at(1)));
//             vSamples.push_back(entry.at(2));
                
            bool NegativeSignal = false;
            if(stol(entry.at(4))<0)NegativeSignal = true;
            
            if(NegativeSignal)vSamples.push_back(-1*stol(entry.at(4)));
            else vSamples.push_back(stol(entry.at(4)));
            
            for(int i=1;i<stoi(entry.at(2));i++)
            {
                vSamples.push_back(stol(entry.at(i+4)));
            }
            
            for(unsigned int i=0;i<vSamples.size()-2;i++)
            {
                std::cout << vSamples.at(i) << std::endl;
                g->SetPoint(g->GetN(),i,vSamples.at(i));
            }
            
            counter++;
            trout->Fill();
        }
    }//everything up to this point is just reading in the waveforms but now we need to look at making proper energy values from them etc *le sigh*
    
    
    
    g->Draw("AP");
    
    trout->Write();
    fout->Close();
    input.close();
    
    printf("Finished waveform conversion :)\n");
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
