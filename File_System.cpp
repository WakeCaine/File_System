#include <iostream>
#include <conio.h>
#include "File_System.h"
 
 
 
File_System::File_System(void)
{
        FilesDescribtionCatalog= new Catalog_Entry [32];
        FileAllocationTable = new FilePosition_Entry [32];
        AllocatedData = new string [32];
        myfileread.open("FAT.txt", ios::in);
        if(myfileread.is_open())
        {
                string temporarystring;
                char* l=new char[2];
                int positionNumber=0;
                //POBIERANIE TABLICY FAT Z PLIKU
                getline(myfileread,temporarystring);

                #pragma warning(disable : 4996)
                for(int i=0; i<32; i++)
                {
                        temporarystring.copy(l, 2, positionNumber);
                        FileAllocationTable[i].ChangeAdress(0,atoi(l));
                        positionNumber+=2;
                        temporarystring.copy(l, 2, positionNumber);
                        FileAllocationTable[i].ChangeAdress(1,atoi(l));
                        if(i<31)
						{
                                positionNumber+=2;
						}
                }
                temporarystring.clear();

                //POBIERANIE WIELKOSCI KATALOGU i odczytywanie wpisów katalogowych
                getline(myfileread,temporarystring);
                int SizeOfCatalog=(int)temporarystring[0];
                SizeCatalog=SizeOfCatalog-48;
                SizeOfCatalog-=48;

                if(SizeOfCatalog>0)
                {
                        for(int i=0; i<SizeOfCatalog; i++)
                        {
                                string tempString;
                                getline(myfileread,temporarystring);
                                temporarystring.copy(l,temporarystring.length()-8,0);
                                tempString.assign(l,temporarystring.length()-8);
                                FilesDescribtionCatalog[i].Name=tempString;
 
                                temporarystring.copy(l,1,temporarystring.length()-8);
                                FilesDescribtionCatalog[i].type=atoi(l);
 
                                temporarystring.copy(l,1,temporarystring.length()-7);
                                FilesDescribtionCatalog[i].flag=l[0];
 
                                temporarystring.copy(l,2,temporarystring.length()-6);
                                tempString.clear();
                                for(int j=0; j<2; j++)tempString+=l[j];
                                FilesDescribtionCatalog[i].location=stoi(tempString);
 
                                temporarystring.copy(l,4,temporarystring.length()-4);
                                FilesDescribtionCatalog[i].size=atoi(l);         
                        }
                }
                //POBIERANIE TABLICY JEDNOSTEK ALOKACJI
                for(int i=0; i<32; i++)
                {
                        getline(myfileread,temporarystring);
                        if(!(temporarystring.length()>32))
						{
                                AllocatedData[i]=temporarystring;
						}
                        else
                        {
                                temporarystring.resize(32);
                                AllocatedData[i]=temporarystring;
                        }
                }
                myfileread.close();
 
                //TWORZENIE LISTY WOLNYCH JEDNOSTEK ALOKACJI
                for(int i=0; i<32; i++)
                {
                        if(FileAllocationTable[i].returnFirstAdress()==32)
						{
                                EmptyBlocksFAT.push_back(i);
						}
                }
        }
}
 
bool File_System::ChangeFlagOfFile(string name, char flag, int& error)
{
        try{
 
        error=0;
        bool passed=false;
        if(flag!='R' && flag!='W')
        {
                error=4;
                return false;
        }
 
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        if(FilesDescribtionCatalog[i].flag==flag)
                        {
                                error=4;
                                return false;
                        }
                        FilesDescribtionCatalog[i].flag=flag;
                        Update();
                        return true;
                }
        }
        error=4;
        Update();
        return passed;
       
        }catch(...){Format(); error=7; return false;}
}
 
bool File_System::ChangeFlagOfFile(int placeInCatalog, char flag, int& error)
{
        error=0;
        if(placeInCatalog>SizeCatalog-1)
        {
                error=4;
                return false;
        }
        else
        {
                return ChangeFlagOfFile(FilesDescribtionCatalog[placeInCatalog].Name, flag, error);
        }
}
 
bool File_System::ViewFile(int placeInCatalog, string& data, int& error)
{
        try{
 
        error=0;
        string datatemp;
        bool check=true;
        if(placeInCatalog>SizeCatalog-1)
        {
                error=5;
                return false;
        }
 
        int place=FilesDescribtionCatalog[placeInCatalog].location;
 
        while(check==true)
        {
                datatemp+=AllocatedData[FileAllocationTable[place].returnFirstAdress()];
                if(FileAllocationTable[place].returnSecondAdress()==32)
                {
                        check=false;
                }
                else
                {
                        int tempplace=place;
                        place=FileAllocationTable[tempplace].returnSecondAdress();
                }
        }
		int placeleft;
        int leg=datatemp.length();
        for(unsigned int i=0; i<datatemp.length(); i++)
        {
            if(datatemp[i]=='~')
            {
				datatemp.resize(i);
                break;
            }
            else
                placeleft=0;
        }
        data=datatemp;
        return true;
 
        }catch(...){Format(); error=7; return false;}
}
 
bool File_System::ViewFile(string name, string& data, int& error)
{
        error=0;
        bool ret=false;
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        ret=ViewFile(i, data, error);
                        return ret;
                }
        }
        error=5;
        return ret;
}
 
bool File_System::ChangeFile(string name, string data, int& error)
{
        bool ret=false;
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        ret=ChangeFile(i, data, error);
                        return ret;
                }
        }
        error=5;
        return ret;
}
 
bool File_System::ChangeFile(int placeInCatalog, string data, int& error)
{
        try{
 
        error = 0;
        if(placeInCatalog > SizeCatalog - 1)
        {
                error = 5;
                return false;
        }
        else if(FilesDescribtionCatalog[placeInCatalog].flag=='R')
        {
                error=6;
                return false;
        }
 
        char* dataCHAR = new char [32];
        for(int i=0; i<32; i++)
		{
                dataCHAR[i] = 0;
		}

        int NeededBlocks = data.length()/32;
        int ExistingBlocks = FilesDescribtionCatalog[placeInCatalog].size/32;
        float checkvaluefloat=(float)FilesDescribtionCatalog[placeInCatalog].size;
        checkvaluefloat/=32;

        if(ExistingBlocks==checkvaluefloat && ExistingBlocks!=0)
        {
                ExistingBlocks-=1;
        }
        int FirstFAT = 32;

        if(FilesDescribtionCatalog[placeInCatalog].size==32)
		{
                ExistingBlocks=0;
		}
        NeededBlocks+= 1;
        ExistingBlocks+= 1;
 
        if(!CheckEmptySpace((NeededBlocks-ExistingBlocks)*32))
        {
                error = 1;
                return false;
        }
 
        int position = FilesDescribtionCatalog[placeInCatalog].location;
        FilesDescribtionCatalog[placeInCatalog].size = data.length();
        int dataPosition = 0;

        if(NeededBlocks == ExistingBlocks)
        {
                bool more = true;
                while(more == true)
                {
                        if((data.length()-dataPosition)<32)
                        {
                                int dataToFill = 32-(data.length()-dataPosition);
                                for(int j=0; j<dataToFill; j++)
                                {
                                        data+="~";
                                }
                        }
                        data.copy(dataCHAR, 32, dataPosition);
                        AllocatedData[FileAllocationTable[position].returnFirstAdress()].assign(dataCHAR,32);
                        if(FileAllocationTable[position].returnSecondAdress() == 32)
                        {
                                more=false;
                        }
                        else
                        {
                                int templocation = position;
                                position = FileAllocationTable[templocation].returnSecondAdress();
                                for(int i = 0; i < 32; i++)
                                dataCHAR[i] = 0;
                                dataPosition+= 32;
                        }
                }
        }      
        else if(NeededBlocks > ExistingBlocks)
        {
                for(int i = 0; i < ExistingBlocks; i++)
                {
                        if((data.length()-dataPosition)<32)
                        {
                                int dataToFill = 32-(data.length()-dataPosition);
                                for(int j=0; j<dataToFill; j++)
                                {
                                        data+="~";
                                }
                        }
                        data.copy(dataCHAR, 32, dataPosition);
                        AllocatedData[FileAllocationTable[position].returnFirstAdress()].assign(dataCHAR,32);
                        int tempposition = position;
                        if(FileAllocationTable[tempposition].returnSecondAdress()!=32)
						{
                                position=FileAllocationTable[tempposition].returnSecondAdress();
						}
                        for(int i = 0; i < 32; i++)
						{
                                dataCHAR[i] = 0;
						}
                        dataPosition+= 32;
                }
 
                FileAllocationTable[position].ChangeAdress(1,EmptyBlocksFAT.front());
 
                for(int i = 0; i < NeededBlocks - ExistingBlocks; i++)
                {
                        position = EmptyBlocksFAT.front();
                        FileAllocationTable[position].ChangeAdress(0,EmptyBlocksFAT.front());
                        if((data.length()-dataPosition)<32)
                        {
                                int dataToFill = 32-(data.length()-dataPosition);
                                for(int j=0; j<dataToFill; j++)
                                {
                                        data+="~";
                                }
                        }
                        data.copy(dataCHAR, 32, dataPosition);
                        AllocatedData[EmptyBlocksFAT.front()].assign(dataCHAR,32);
                        EmptyBlocksFAT.pop_front();
                        if(!(i + 1 == NeededBlocks - ExistingBlocks))
						{
                                FileAllocationTable[position].ChangeAdress(1,EmptyBlocksFAT.front());
						}
                        if(NeededBlocks > 1)
                        {
                                for(int i = 0; i < 32; i++)
                                        dataCHAR[i] = 0;
                                dataPosition+= 32;
                        }
                }
        }
        else if(NeededBlocks < ExistingBlocks)
        {
                int nextposition;
                for(int i = 0; i < NeededBlocks; i++)
                {
                        if((data.length()-dataPosition)<32)
                        {
                                int dataToFill = 32-(data.length()-dataPosition);
                                for(int j=0; j<dataToFill; j++)
                                {
                                        data+="~";
                                }
                        }
                        data.copy(dataCHAR, 32, dataPosition);
                        AllocatedData[FileAllocationTable[position].returnFirstAdress()].assign(dataCHAR,32);
                        try{
                        AllocatedData[FileAllocationTable[position].returnFirstAdress()].erase(data.length(), 32);
                        }catch(...){}
                        int tempposition = position;
                        nextposition = position;
                        position = FileAllocationTable[tempposition].returnSecondAdress();
                       
                        for(int i = 0; i < 32; i++)
						{
                                dataCHAR[i] = 0;
						}
                        dataPosition+=32;
                }
 
                FileAllocationTable[nextposition].ChangeAdress(1,32);
               
                for(int i = 0; i < ExistingBlocks - NeededBlocks; i++)
                {
                        AllocatedData[FileAllocationTable[position].returnFirstAdress()].assign("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
                        FileAllocationTable[position].ChangeAdress(0,32);
                        int tempposition = position;
                        position = FileAllocationTable[tempposition].returnSecondAdress();
                        FileAllocationTable[tempposition].ChangeAdress(1,32);
                }
        }
       
        error = 0;
        EmptyBlocksFAT.clear();
        for(int i=0; i<32; i++)
                {
                        if(FileAllocationTable[i].returnFirstAdress()==32)
                                EmptyBlocksFAT.push_back(i);
                }
        Update();
        return true;
 
        }catch(...){Format(); error=7; return false;}
}
 
bool File_System::CreateNewFile(string name, int type, char flag, string data, int& error)
{
        try{
 
        error=0;
        bool empty=CheckEmptySpace(name.length());
        if(empty)
        {
                return AllocateData(name, type, flag, data, error);
        }
        else
        {
                error=1;
                return false;
        }
 
        }catch(...){Format(); error=7; return false;}
}
 
bool File_System::CheckEmptySpace(int length)
{
        int AlaviableSpace=0;
        if(!EmptyBlocksFAT.empty())
        {
                AlaviableSpace=EmptyBlocksFAT.size()*32;
        }
 
        if(AlaviableSpace>=length)
                return true;
        else
                return false;
}
 
bool File_System::AllocateData(string name, int type, char flag, string data, int& error)
{
        error=0;
        if(name.length()>8)
        {
                error=2;
                return false;
        }
        for(int i=0; i<SizeCatalog; i++)
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        error=3;
                        return false;
                }
 
        if(flag!='R' && flag!='W')
        {
                error=4;
                return false;
        }
 
 
        char* dataCHAR=new char [32];
        for(int i=0; i<32; i++)
                        dataCHAR[i]=0;
        int NeededBlocks=data.length()/32;
        int FirstFAT=32;
        NeededBlocks+=1;
        int dataPositon=0;
        FirstFAT=EmptyBlocksFAT.front();
 
        try{
        FilesDescribtionCatalog[SizeCatalog].Name=name;
        FilesDescribtionCatalog[SizeCatalog].type=type;
        FilesDescribtionCatalog[SizeCatalog].flag=flag;
        FilesDescribtionCatalog[SizeCatalog].size=data.length();
        FilesDescribtionCatalog[SizeCatalog].location=FirstFAT;
        SizeCatalog+=1;}
        catch(exception a)
        {
                error=4;
                return false;
        }
 
        for(int i=0; i<NeededBlocks; i++)
        {
                int position=EmptyBlocksFAT.front();
                FileAllocationTable[position].ChangeAdress(0,EmptyBlocksFAT.front());
                if((data.length()-dataPositon)<32)
                {
                        int dataToFill = 32-(data.length()-dataPositon);
                        for(int j=0; j<dataToFill; j++)
                        {
                                data+="~";
                        }
                }
                data.copy(dataCHAR, 32, dataPositon);
               
                AllocatedData[EmptyBlocksFAT.front()].assign(dataCHAR,32);
                EmptyBlocksFAT.pop_front();
                if(!(i+1==NeededBlocks))
				{
                        FileAllocationTable[position].ChangeAdress(1,EmptyBlocksFAT.front());
				}
                if(NeededBlocks>1)
                {
                        for(int i=0; i<32; i++)
						{
                                dataCHAR[i]=0;
						}
                        dataPositon+=32;
                }
        }
        Update();
        return true;
}
 
bool File_System::AppendFile(string name, string data, int& error)
{
        bool ret=false;
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        ret=AppendFile(i, data, error);
                        return ret;
                }
        }
        error=5;
        return ret;
}
 
bool File_System::AppendFile(int placeInCatalog, string data, int& error)
{
        try{
 
        error=0;
        if(FilesDescribtionCatalog[placeInCatalog].flag=='R')
        {
                error=6;
                return false;
        }
 
        bool loop=true;
        int templocation = FilesDescribtionCatalog[placeInCatalog].location;
        while(loop==true)
        {
                if(FileAllocationTable[templocation].returnSecondAdress() == 32)
                {
                        if(AllocatedData[FileAllocationTable[templocation].returnFirstAdress()].size()+data.length()<=32)
                        {
                                AllocatedData[FileAllocationTable[templocation].returnFirstAdress()]+=data;
                               
                                FilesDescribtionCatalog[placeInCatalog].size+=data.length();
                               
                        }
                        else if(AllocatedData[FileAllocationTable[templocation].returnFirstAdress()].size()+data.length()>32)
                        {
                                int FatAlloc;
                                string TempString=AllocatedData[FileAllocationTable[templocation].returnFirstAdress()];
                                bool anotherloop=true;
                                int dataPositon=0;
                                char* dataCHAR=new char [32];
                                for(int i=0; i<32; i++)
								{
                                        dataCHAR[i]=0;
								}
 
                                unsigned int placeleft;
                                for(int i=dataPositon; i<(dataPositon+32); i++)
                                {
                                        if(TempString[i]=='~')
                                        {
                                                placeleft=32-i;
                                                        break;
                                        }
                                        else
                                                placeleft=0;
                                }
                                string modifestring;
                                if(placeleft>0)
                                {
                                        modifestring=data;
                                        if(placeleft<modifestring.length())
										{
                                                modifestring.resize(placeleft);
										}
                                        AllocatedData[FileAllocationTable[templocation].returnFirstAdress()].resize(32-placeleft);
                                        if((modifestring.length()-dataPositon)<32)
                                        {
                                                int dataToFill = 32-(modifestring.length()-dataPositon);
                                                for(int j=0; j<dataToFill; j++)
                                                {
                                                        modifestring+="~";
                                                }
                                        }
                                        AllocatedData[FileAllocationTable[templocation].returnFirstAdress()]+=modifestring;
                                        modifestring=data;
                                        int FirstFAT=32;
                                        modifestring.erase(0,placeleft);
                                        FatAlloc=modifestring.length()/32;
                                        if(CheckEmptySpace(modifestring.length())==false)
										{
                                                return false;
										}
                                        if(modifestring.length()!=0)
                                        {
												FatAlloc+=1;
												FileAllocationTable[templocation].ChangeAdress(1,EmptyBlocksFAT.front());
												for(int i=0; i<FatAlloc; i++)
                                                {
                                                        int position=EmptyBlocksFAT.front();
                                                       
                                                        if(i==0)
                                                        {
                                                                FirstFAT=position;
                                                        }
                                                        FileAllocationTable[position].ChangeAdress(0,EmptyBlocksFAT.front());
                                                       
                                                        if((modifestring.length()-dataPositon)<32)
                                                        {
                                                                int dataToFill = 32-(modifestring.length()-dataPositon);
                                                                for(int j=0; j<dataToFill; j++)
                                                                {
                                                                        modifestring+="~";
                                                                }
                                                        }
                                                        modifestring.copy(dataCHAR, 32, dataPositon);
               
                                                        AllocatedData[EmptyBlocksFAT.front()].assign(dataCHAR,32);
                                                        EmptyBlocksFAT.pop_front();
                                                        if(!(i+1==FatAlloc))
                                                                FileAllocationTable[position].ChangeAdress(1,EmptyBlocksFAT.front());
                                                        if(FatAlloc>1)
                                                        {
                                                                for(int i=0; i<32; i++)
                                                                        dataCHAR[i]=0;
                                                                dataPositon+=32;
                                                        }
                                                }
                                        }
                                        FilesDescribtionCatalog[placeInCatalog].size+=data.length();
                                        Update();
                                        return true;
                                       
                                }
                                else
                                {
                                        modifestring=data;
                                        int FirstFAT=32;
                                        modifestring.erase(0,placeleft);
                                        FatAlloc=modifestring.length()/32;
                                        if(CheckEmptySpace(modifestring.length())==false)
                                                return false;
                                        FatAlloc+=1;
                                        FileAllocationTable[templocation].ChangeAdress(1,EmptyBlocksFAT.front());
                                        for(int i=0; i<FatAlloc; i++)
                                                {
                                                        int position=EmptyBlocksFAT.front();
                                                       
                                                        if(i==0)
                                                        {
                                                                FirstFAT=position;
                                                        }
                                                        FileAllocationTable[position].ChangeAdress(0,EmptyBlocksFAT.front());
                                                        data.copy(dataCHAR, 32, dataPositon);
               
                                                        AllocatedData[EmptyBlocksFAT.front()].assign(dataCHAR,32);
                                                        try{
                                                        AllocatedData[EmptyBlocksFAT.front()].erase(data.length(), 32);
                                                        }catch(...){}
                                                        EmptyBlocksFAT.pop_front();
                                                        if(!(i+1==FatAlloc))
                                                                FileAllocationTable[position].ChangeAdress(1,EmptyBlocksFAT.front());
                                                        if(FatAlloc>1)
                                                        {
                                                                for(int i=0; i<32; i++)
                                                                        dataCHAR[i]=0;
                                                                dataPositon+=32;
                                                        }
                                                }
                                        FilesDescribtionCatalog[placeInCatalog].size+=data.length();
                                        Update();
                                        return true;
                                }
                        }
                        break;
                }
                else
                {
                        templocation=FileAllocationTable[templocation].returnSecondAdress();
                }
        }
        return false;
        }catch(...){Format(); error=7; return false;}
       
}
 
bool File_System::RenameFile(int placeInCatalog, string NameReplacement, int& error)
{
        if(placeInCatalog>SizeCatalog)
        {
                error=4;
                return false;
        }
        else
                return RenameFile(FilesDescribtionCatalog[placeInCatalog-1].Name, NameReplacement, error);
}
 
bool File_System::RenameFile(string name, string NameReplacement, int& error)
{
        error=0;
        if(NameReplacement.length()>8)
        {
                error=2;
                return false;
        }
 
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        if(FilesDescribtionCatalog[i].Name==NameReplacement)
                        {
                                error=3;
                                return false;
                        }
                        FilesDescribtionCatalog[i].Name=NameReplacement;
                        Update();
                        return true;
                }
        }
        error=4;
        return false;
}
 
bool File_System::DeleteFile(int placeInCatalog, int& error)
{
        return DeleteFile(FilesDescribtionCatalog[placeInCatalog].Name, error);
}
 
bool File_System::DeleteFile(string name, int& error)
{
        try{
        error=0;
        bool found=false;
        for(int i=0; i<SizeCatalog; i++)
        {
                if(FilesDescribtionCatalog[i].Name==name)
                {
                        int location=FilesDescribtionCatalog[i].location;
                        bool more=true;
                        int place=i;
                        while(more==true)
                        {
                                string FillingString="~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
                                AllocatedData[FileAllocationTable[location].returnFirstAdress()].assign(FillingString);
                                FileAllocationTable[location].ChangeAdress(0,32);
                                if(FileAllocationTable[location].returnSecondAdress()==32)
                                {
                                        more=false;
                                }
                                else
                                {
                                        int templocation=location;
                                        location=FileAllocationTable[templocation].returnSecondAdress();
                                        FileAllocationTable[templocation].ChangeAdress(1,32);
                                }
                        }
 
                        for(int j=i; j<SizeCatalog-1; j++)
                        {
                                int tempj=j+1;
                                FilesDescribtionCatalog[j].Name=FilesDescribtionCatalog[tempj].Name;
                                FilesDescribtionCatalog[j].type=FilesDescribtionCatalog[tempj].type;
                                FilesDescribtionCatalog[j].flag=FilesDescribtionCatalog[tempj].flag;
                                FilesDescribtionCatalog[j].location=FilesDescribtionCatalog[tempj].location;
                                FilesDescribtionCatalog[j].size=FilesDescribtionCatalog[tempj].size;
                        }
                        found=true;
                        SizeCatalog-=1;
                        break;
                }
                else found=false;
        }
       
        if(found=true)
        {
                error=0;
                EmptyBlocksFAT.clear();
                for(int i=0; i<32; i++)
                {
                        if(FileAllocationTable[i].returnFirstAdress()==32)
                                EmptyBlocksFAT.push_back(i);
                }
                Update();
                return true;
        }
        else
        {
                error=5;
                return false;
        }
 
        }catch(...){Format(); error=7; return false;}
}
 
bool File_System::UpdateDataToFile()
{
        string temporarystring;
        myfile.open("FAT.txt");
        if(myfile.is_open())
        {
                string temporarystring;
                char* l=new char[2];
                int positionNumber=0;
                //POBIERANIE TABLICY FAT i Zapis do pliku
                for(int i=0; i<32; i++)
                {
                        if(FileAllocationTable[i].returnFirstAdress()<10)
                                myfile<<"0"+to_string(FileAllocationTable[i].returnFirstAdress());
                        else
                                myfile<<FileAllocationTable[i].returnFirstAdress();
                        if(FileAllocationTable[i].returnSecondAdress()<10)
                                myfile<<"0"+to_string(FileAllocationTable[i].returnSecondAdress());
                        else
                                myfile<<FileAllocationTable[i].returnSecondAdress();
                }
                myfile<<"\n";
                //POBIERANIE WIELKOSCI KATALOGU i odczytywanie wpisów katalogowych do PLIKU
				std::cout<< SizeCatalog<<std::endl; 

                myfile<<SizeCatalog<<"\n";
                if(SizeCatalog>0)
                {
                        for(int i=0; i<SizeCatalog; i++)
                        {
                                myfile<<FilesDescribtionCatalog[i].Name;
                                myfile<<to_string(FilesDescribtionCatalog[i].type);
                                myfile<<FilesDescribtionCatalog[i].flag;
 

								std::cout<< FilesDescribtionCatalog[i].Name <<std::endl; 
								std::cout<< to_string(FilesDescribtionCatalog[i].type) << std::endl; 
								std::cout<< FilesDescribtionCatalog[i].flag <<std::endl; 


                                if(FilesDescribtionCatalog[i].location<10)
                                        myfile<<"0"+to_string(FilesDescribtionCatalog[i].location);
                                else
                                        myfile<<to_string(FilesDescribtionCatalog[i].location);
 
                                if(FilesDescribtionCatalog[i].size<10)
                                        myfile<<"000"+to_string(FilesDescribtionCatalog[i].size)<<"\n";
                                else if(FilesDescribtionCatalog[i].size<100 && FilesDescribtionCatalog[i].size>=10 )
                                        myfile<<"00"+to_string(FilesDescribtionCatalog[i].size)<<"\n";
                                else if(FilesDescribtionCatalog[i].size<1000 && FilesDescribtionCatalog[i].size>=100)
                                        myfile<<"0"+to_string(FilesDescribtionCatalog[i].size)<<"\n";
                        }
                }
                //POBIERANIE TABLICY JEDNOSTEK ALOKACJI do Pliku
                for(int i=0; i<32; i++)
                {
                        if(!(AllocatedData[i].length()>32))
                                myfile<<AllocatedData[i]<<"\n";
                        else
                        {
                                AllocatedData[i].resize(32);
                                myfile<<AllocatedData[i]<<"\n";
                        }
                }
                myfile<<"\n";
                myfile.close();
        }
        return true;
}
 
bool File_System::UpdateDataFromFile()
{
        myfileread.open("FAT.txt", ios::in);
        if(myfileread.is_open())
        {
                string temporarystring;
                char* l=new char[2];
                int positionNumber=0;
                //POBIERANIE TABLICY FAT Z PLIKU
                getline(myfileread,temporarystring);
                #pragma warning(disable : 4996)
                for(int i=0; i<32; i++)
                {
                        temporarystring.copy(l, 2, positionNumber);
                        FileAllocationTable[i].ChangeAdress(0,atoi(l));
                        positionNumber+=2;
                        temporarystring.copy(l, 2, positionNumber);
                        FileAllocationTable[i].ChangeAdress(1,atoi(l));
                        if(i<31)
                                positionNumber+=2;
                }
                temporarystring.clear();
                //POBIERANIE WIELKOSCI KATALOGU i odczytywanie wpisów katalogowych
                getline(myfileread,temporarystring);
                int SizeOfCatalog=(int)temporarystring[0];
                SizeCatalog=SizeOfCatalog-48;
                SizeOfCatalog-=48;
                if(SizeOfCatalog>0)
                {
                        for(int i=0; i<SizeOfCatalog; i++)
                        {
                                string tempString;
                                getline(myfileread,temporarystring);
                                temporarystring.copy(l,temporarystring.length()-8,0);
                                tempString.assign(l,temporarystring.length()-8);
                                FilesDescribtionCatalog[i].Name=tempString;
                                temporarystring.copy(l,1,temporarystring.length()-8);
                                FilesDescribtionCatalog[i].type=atoi(l);
 
                                temporarystring.copy(l,1,temporarystring.length()-7);
                                FilesDescribtionCatalog[i].flag=l[0];
 
                                temporarystring.copy(l,2,temporarystring.length()-6);
                                tempString.clear();
                                for(int j=0; j<2; j++)tempString+=l[j];
									FilesDescribtionCatalog[i].location=stoi(tempString);
 
                                temporarystring.copy(l,4,temporarystring.length()-4);
                                FilesDescribtionCatalog[i].size=atoi(l);
                               
                        }
                }
 
                //POBIERANIE TABLICY JEDNOSTEK ALOKACJI
                for(int i=0; i<32; i++)
                {
                        getline(myfileread,temporarystring);
                        if(!(temporarystring.length()>32))
                                AllocatedData[i]=temporarystring;
                        else
                        {
                                temporarystring.resize(32);
                                AllocatedData[i]=temporarystring;
                        }
                }
                myfileread.close();
 
                //TWORZENIE LISTY WOLNYCH JEDNOSTEK ALOKACJI
                for(int i=0; i<32; i++)
                {
                        if(FileAllocationTable[i].returnFirstAdress()==32)
                                EmptyBlocksFAT.push_back(i);
                }
        }
        return true;
}
 
bool File_System::Format()
{
        try{
        for(int i=1; i<32; i++)
        {
                FileAllocationTable[i].ChangeAdress(0,32);
                FileAllocationTable[i].ChangeAdress(1,32);
                SizeCatalog=1;
                AllocatedData[i].assign("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        }
        Update();
        }catch(...){ return false;}
        return true;
}
 
string* File_System::ListOfFiles(int& sizeOfCatalog)
{
        try{
 
        string* list=new string [SizeCatalog];
        sizeOfCatalog=SizeCatalog;
        for(int i=0; i<SizeCatalog; i++)
        {
                string extension;
                if(FilesDescribtionCatalog[i].type==0)
                        extension=".exe";
                else
                        extension=".txt";
                list[i]=FilesDescribtionCatalog[i].Name+extension+" "+FilesDescribtionCatalog[i].flag;
        }
        return list;
 
        }catch(...){Format(); return false;}
}
 
void File_System::ShowFAT()
{
        int counter=0;
        cout<<"\n"<<"File Allocation Table:"<<endl;
        for(int i=0; i<32; i++)
        {
            counter+=1;
            cout<<i<<":"<<FileAllocationTable[i].returnFirstAdress()<<"|"<<FileAllocationTable[i].returnSecondAdress()<<" \t";
            if(counter==4)
            {
                cout<<"\n";
                counter=0;
            }    
        }
 }

File_System::~File_System(void)
{
        UpdateDataToFile();
        delete[] FileAllocationTable, FilesDescribtionCatalog, AllocatedData;
}

