#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm> 

using namespace std;

class File_System
{
public:
	File_System(void);
	~File_System(void);
private:

	class Catalog_Entry
	{
	public:
		string Name;
		int type;
		char flag;
		int location;
		int size;
	};
	//Klasa wpisu w katalogu
	
	class FilePosition_Entry
	{
	private:
		int First_adress;
		int Next_adress;
	public:
		FilePosition_Entry(void){};
		~FilePosition_Entry(void){};
		int returnFirstAdress()
		{
			return First_adress;
		}
		int returnSecondAdress()
		{
			return Next_adress;
		}
		void ChangeAdress(int i, int Adress)
		{
			if(i==0)
				First_adress=Adress;
			else
				Next_adress=Adress;
		}
	};
	//Klasa wpisu w tablicy FAT

	int SizeCatalog; //Wielkoœæ katalogu. Wiêksza od 1, jak istnieje wpis.
	ofstream myfile; //Zmienna do odczytywania z pliku tekstowego
	ifstream myfileread; //Zmienna do zapisu do pliku tekstowego
	std::list<int> EmptyBlocksFAT; //Lista wolnych jednostek alokacji
	Catalog_Entry* FilesDescribtionCatalog; //Tablica wpisów katalogowych
	FilePosition_Entry* FileAllocationTable; //Tablica FAT - 32 wpisy 
	string* AllocatedData; //Tablica jednostek alokacji plików- 32 wpisy po 32 bajty
	bool Update()
	{
		UpdateDataToFile();
		UpdateDataFromFile();
		return true;
	}
	//Zapis do pliku i odczyt z pliku
	bool UpdateDataToFile();
	bool UpdateDataFromFile();

	bool AllocateData(string name, int type, char flag, string data, int& error);
	//Rezerwacja i zapis danych do jednostek alokacji

public:

	bool Format();
	//Formatowanie dysku
	bool CheckEmptySpace(int length);
	//Przy podaniu d³ugoœci danych sprawdza, czy jest wystarczaj¹co miejsca

	bool CreateNewFile(string name, int type, char flag, string data, int& error);
	/*Tworzenie nowego pliku:
	name-Nazwa maks. 8 liter/cyfr
	type- rozszerzenie 0=exe, 1=txt
	flag- dwie flagi dostêpu do wyboru R i W (opisane ni¿ej)
	data- dane w formie stringa do zapisania(jednostka alokacji po 32 bajty)
	int& error- zwraca numer b³êdu(opisane poni¿ej)*/

	bool ReturnName(int placeInCatalog, int& error, string& name)
	{
		if(SizeCatalog-1<placeInCatalog)
		{
			error=5;
			return false;
		}
		else 
		{
			name=FilesDescribtionCatalog[placeInCatalog].Name;
			return true;
		}
	}
	//Zwraca nazwê pliku w string name
	bool ReturnSize(int placeInCatalog, int& error, int& size)
	{
		if(SizeCatalog<placeInCatalog)
		{
			error=5;
			return false;
		}
		else 
		{
			string datatemp;
			ViewFile(placeInCatalog,datatemp,error);
        for(unsigned int i=0; i<datatemp.length(); i++)
        {
                if(datatemp[i]=='~')
                        {
                                datatemp.resize(i);
                                        break;
				}
		}
		size=datatemp.size();
			return true;
		}
	}
	//Zwraca rozmiar pliku w int size (rozmiar ca³ego stringa)
	bool ReturnSize(string name, int& error, int& size)
	{
		for(int i=0; i<SizeCatalog; i++)
		{
			if(FilesDescribtionCatalog[i].Name==name)
				return ReturnSize(i, error, size);
		}
		return false;
	}
	//Zwraca rozmiar pliku w int size (rozmiar ca³ego stringa)
	bool ReturnFlag(int placeInCatalog, int& error, char& flag)
	{
		error=0;
		if(SizeCatalog-1<placeInCatalog)
		{
			error=5;
			return false;
		}
		else 
		{
			flag=FilesDescribtionCatalog[placeInCatalog].flag;
			return true;
		}
	}
	//Zwraca flagê pliku w char flag, R lub W(opisane poni¿ej)
	bool ReturnType(int placeInCatalog, int& error, int& type)
	{
		error=0;
		if(SizeCatalog-1<placeInCatalog)
		{
			error=5;
			return false;
		}
		else 
		{
			type=FilesDescribtionCatalog[placeInCatalog].type;
			return true;
		}
	}
	//Zwraca typ pliku w int type(rozszerzenie), 0=exe, 1=txt

	bool ChangeFlagOfFile(string name, char flag, int& error);
	bool ChangeFlagOfFile(int placeInCatalog, char flag, int& error);
	//Pozwala na zmianê flagi pliku

	bool ViewFile(int placeInCatalog, string& data, int& error);
	bool ViewFile(string name, string& data, int& error);
	//Wyœwietlanie- do data zwraca ca³y string z danymi

	bool ChangeFile(string name, string data, int& error);
	bool ChangeFile(int placeInCatalog, string data, int& error);
	/*Zmiana pliku- data to string, który ma zmieniæ ten istniej¹cy
	w przypadku, gdy jest mniejszy, ni¿ obecny NIE ZOSTANIE NADPISANY,
	lecz usuniêty i zmieniony.*/

	bool AppendFile(string name, string data, int& error);
	bool AppendFile(int placeInCatalog, string data, int& error);
	//Dopisywanie danych na koñcu istniej¹cego pliku.

	bool RenameFile(string name, string NameReplacement, int& error);
	bool RenameFile(int placeInCatalog, string NameReplacement, int& error);
	//Zmiana nazwy- w przypadku podania tej samej zwraca b³¹d

	string* ListOfFiles(int& sizeOfCatalog);
	//Zwraca listê plików z katalogu wraz z ich flag¹ w formie stringa

	bool DeleteFile(string name, int& error);
	bool DeleteFile(int placeInCatalog, int& error);
	void File_System::ShowFAT();
	//Usuwanie plików
};

/*W przypadku powy¿szych funkcji nale¿y pamiêtaæ,
	¿e katalog ma numeracjê od 0 do (RozmiarKatlogu-1)
	---------------------------------------------------------------
	Wszystkie zmienne z & oddaj¹ wartoœci, 
	wiêc powinny mieæ zadeklarowane zmienne, do których moga je oddaæ
	---------------------------------------------------------------
	FLAGI(char flag):
	'R'-Tylko do odczytywania- nie mo¿na zmieniaæ/dopisywaæ
	'W'-Odczytywanie i zmiana
	---------------------------------------------------------------
	ERROR(int& error):
	0-Brak error
	1-Brak miejsca
	2-Zbyt d³uga nazwa
	3-Nazwa ju¿ istnieje
	4-B³¹d atrybutów
	5-Z³e dane(nie ma takiego pliku etc.)
	6-Plik o ograniczonym prawie dostêpu- nie mo¿na zapisaæ/zmieniaæ
	7-Crash, dysk zostaje wyczyszczony w celu zapobiegniêcia innym b³êdom
	---------------------------------------------------------------
	------------------------*UWAGA*--------------------------------
	Plik FAT.txt do³¹czony do archiwum musi znajdowaæ siê tam, 
	gdzie jest wykonywalny lub debugowany .exe programu.
	W innym przypadku system plików nie bêdzie sprawnie dzia³a³.
	*/
