#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <conio.h>
#include <time.h>
#define max 1000
#define maks(a,b) (a > b ? a : b) //max z (a,b), a>b -> return a, else -> return b
#define min(a,b) (a > b ? b : a)
using namespace std;

int liczbaprzedmiotow; //rozmiar tablicy z przedmiotami
int pojemnoscplecaka; //max. waga plecaka
int waga[max];		//tablica wag
int wartosc[max];	//tablica wartosci
float wspprzedmiotu[max];	//tablica wspolczynnikow (wartosc/waga)
int i, j;					//indeksy pomocnicze do petli
int tmp[max];				//pomocnicza tablica do przechowywania wektora 0/1 (plecak[max])
int plecak[max]; // if == 1 -> przedmiot w plecaku, == 0 -> nie
float zyskkoncowy = -1;
int wagakoncowa;

//programowanie dynamiczne
float wyniki[max][max] = { 0 };
unsigned int przedmioty[max][max] = { 0 };

//FPTAS
int **wynikiFPTAS;
int wartosc2[max];
float epsilon, K;
int temp = -1;

//FUNKCJE POMOCNICZE
LARGE_INTEGER startTimer()
{
	LARGE_INTEGER start;
	DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&start);
	SetThreadAffinityMask(GetCurrentThread(), oldmask);
	return start;
}

LARGE_INTEGER endTimer()
{
	LARGE_INTEGER stop;
	DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&stop);
	SetThreadAffinityMask(GetCurrentThread(), oldmask);
	return stop;
}

LARGE_INTEGER performanceCountStart, performanceCountEnd;
LARGE_INTEGER freq;

//ZEROWANIE ZMIENNYCH WYKORZYSTYWANYCH WIELOKROTNIE W KAZDYM Z ALGORYTMOW
void wyczysc()
{
	//W razie gdyby wykonywano wiele algorytmow
	if (temp != 0 && liczbaprzedmiotow != 0)
	{
		for (int i = 0; i < liczbaprzedmiotow; i++)
		{
			for (int j = 0; j < liczbaprzedmiotow*temp; j++)
			{
				wynikiFPTAS[i][j] = 0;
			}
		}
	}

	wagakoncowa = 0;
	zyskkoncowy = 0;
	epsilon = K = 0;
	temp = -1;
	for (int i = 0; i < max; i++)
	{
		plecak[i] = tmp[i] = 0;
		for (int j = 0; j < max; j++)
		{
			wyniki[i][j] = przedmioty[i][j] = 0;
		}
	}
	//---------------------------------------------------------------------
}

//ODCZYT Z PLIKU
void zpliku(string nazwa)
{
	//W razie gdyby wykonywano wiele algorytmow
	liczbaprzedmiotow = pojemnoscplecaka = wagakoncowa = i = j = 0;
	zyskkoncowy = 0;
	epsilon = K = 0;
	temp = -1;
	for (int i = 0; i < max; i++)
	{
		waga[i] = wartosc[i] = plecak[i] = tmp[i] = wspprzedmiotu[i] = 0;
		for (int j = 0; j < max; j++)
		{
			wyniki[i][j] = przedmioty[i][j] = 0;
		}
	}
	//---------------------------------------------------------------------
	fstream plik;

	plik.open(nazwa, std::ios::in);
	int i;
	plik >> liczbaprzedmiotow;
	for (i = 0; i < liczbaprzedmiotow; i++)
	{
		plik >> waga[i];
		plik >> wartosc[i];
	}
	plik.close();
	cout << "\nDane z pliku wczytano.";
	cout << "\nPojemnosc plecaka: ";
	cin >> pojemnoscplecaka;
	system("cls");
}

//WYNIK KONCOWY
void wyswietl() //wynik koncowy
{
	cout << "Zawartosc plecaka: " << endl << endl;
	cout << "Waga\t Wartosc\t Wspolczynnik" << endl;

	int wynik = 0;
	int wagak = 0;

	for (i = 0; i < liczbaprzedmiotow; i++)
		if (plecak[i] == 1) cout << waga[i] << "\t" << wartosc[i] << "\t\t" << wspprzedmiotu[i] << endl; // == 1 => w plecaku

	for (i = 0; i < liczbaprzedmiotow; i++)
		if (plecak[i] == 1) // to samo co wyzej
		{
		wynik += wartosc[i] * plecak[i];
		wagak += waga[i];
		}

	cout << "\nMaksymalny zysk: " << wynik << ", waga plecaka: " << wagak << endl;
}

//SORTOWANIE BABELKOWE, WG. WSPOLCZYNNIKA
void sortuj() //babelkowe
{
	int a;
	int b;
	float c;
	for (i = 0; i<liczbaprzedmiotow; i++)
		wspprzedmiotu[i] = (float)wartosc[i] / (float)waga[i];
	for (i = 0; i<liczbaprzedmiotow - 1; i++)
	{
		for (j = i + 1; j<liczbaprzedmiotow; j++)
		{
			if (wspprzedmiotu[i]  < wspprzedmiotu[j])
			{
				c = wspprzedmiotu[i];
				wspprzedmiotu[i] = wspprzedmiotu[j];
				wspprzedmiotu[j] = c;
				a = wartosc[i];
				wartosc[i] = wartosc[j];
				wartosc[j] = a;
				b = waga[i];
				waga[i] = waga[j];
				waga[j] = b;
			}
		}
	}
}

//WYSWIETLANIE WCZYTANEGO/WYGENEROWANEGO PROBLEMU DO ROZWIAZANIA
void struktura()
{
	if (liczbaprzedmiotow == 0) cout << "Brak przedmiotow do wyswietlenia" << endl;
	else
	{
		cout << "Pojemnosc plecaka: " << pojemnoscplecaka << endl;
		cout << "Liczba przedmiotow: " << liczbaprzedmiotow << endl;
		cout << "Waga \t Wartosc \t" << endl;

		for (int i = 0; i < liczbaprzedmiotow; i++)
		{
			cout << waga[i] << "\t" << wartosc[i] << endl;
		}
	}
	_getch();
	system("cls");
}

//BRANCH & BOUND
float bound(float aktzysk, float aktwaga, int k) //Bound. aktualna waga i zysk oraz indeks przedmiotu
{
	float a = aktzysk;
	float b = aktwaga;
	for (i = k; i <= liczbaprzedmiotow; i++) //zachlanne dodawanie przedmiotow
	{
		b = b + waga[i];
		if (b < pojemnoscplecaka) //dopoki dodanie kolejnego przedmiotu jest mozliwe
			a = a + wartosc[i];
		else
			return (a + (1 - (b - pojemnoscplecaka) / (float)waga[i])*wartosc[i]);
	}
	return a; //zwraca maksymalny zysk
}

void doplecaka(int k, float aktzysk, float aktwaga)
{
	if (aktwaga + waga[k] <= pojemnoscplecaka) //jezeli dodanie nowego przedmiotu jest mozliwe
	{
		tmp[k] = 1;
		if (k <= liczbaprzedmiotow) //sprawdzenie czy sa jakiekolwiek przedmioty do wlozenia (rozmiar tablicy rowny 0?)
			doplecaka(k + 1, aktzysk + wartosc[k], aktwaga + waga[k]); //jezeli tak to dodawany jest kolejny przedmiot

		if (((aktzysk + wartosc[k]) > zyskkoncowy) && (k == liczbaprzedmiotow)) //jezeli dodanie ostatniego przedmiotu zwieksza zysk
		{ //to dodawane sa wartosc i waga do ostatecznego wyniku
			zyskkoncowy = aktzysk + wartosc[k];
			wagakoncowa = aktwaga + waga[k];
			for (j = 0; j <= k; j++)
				plecak[j] = tmp[j]; // i mozna przepisac wektor z tablicy tymczasowej do wynikowej
		}
	}
	if (bound(aktzysk, aktwaga, k) >= zyskkoncowy) //(bound wiekszy od dotychczasowego zysku)
	{
		tmp[k] = 0; //przedmiot poza plecakiem
		if (k <= liczbaprzedmiotow)
			doplecaka(k + 1, aktzysk, aktwaga); //a wkladany jest kolejny przedmiot, o ile to mozliwe
		if ((aktzysk > zyskkoncowy) && (k == liczbaprzedmiotow)) //jezeli zysk przy ostatnim przedmiocie jest od dotychczasowego
		{ //to przygotowywane sa wartosci do ostatecznego wyniku
			zyskkoncowy = aktzysk;
			wagakoncowa = aktwaga;
			for (j = 0; j <= k; j++)
				plecak[j] = tmp[j];
		}
	}
}
//---------------------------------------------------------------------------

//PROGRAMOWANIE DYNAMICZNE
void dynamiczne(int ileprzedmiotow, int rozmiar)
{
	int i, j;

	for (i = 1; i <= ileprzedmiotow; i++){ //i - indeksy przedmiotow
		for (j = 0; j <= rozmiar; j++){ //j - wagi przedmiotow
			if (waga[i - 1] <= j){ //jezeli przedmiot i-1 miesci sie w plecaku o wadze j 
				wyniki[i][j] = maks(wyniki[i - 1][j], wartosc[i - 1] + wyniki[i - 1][j - waga[i - 1]]); //do tablicy wynikowej wpisywany jest max(wynik z podproblemu, wynik ze sprawdzanym przedmiotem)
				if (wartosc[i - 1] + wyniki[i - 1][j - waga[i - 1]]>wyniki[i - 1][j]) //wynik lepszy z przedmiotem
					przedmioty[i][j] = 1; //przedmiot w plecaku
				else
					przedmioty[i][j] = 0;
			}
			else{ //przedmiot i-1 nie miesci sie w plecaku
				przedmioty[i][j] = 0;
				wyniki[i][j] = wyniki[i - 1][j]; //przypisywana jest wartosc z podproblemu o indeksie przedmiotu - 1
			}
		}
	}
	cout << wyniki[ileprzedmiotow][rozmiar] << endl;
}

void wyswietldynamiczne() //wybor - dla ktorego algorytmu zostalo wywolane wyswietlanie
{
	cout << "Zawartosc plecaka: " << endl << endl;
	cout << "Waga\t Wartosc\t Wspolczynnik\t" << endl;

	for (int i = 0; i < liczbaprzedmiotow; i++)
		wspprzedmiotu[i] = (float)wartosc[i] / (float)waga[i];

	int tempwaga = pojemnoscplecaka;
	int indeks = liczbaprzedmiotow;
	int wagak = 0;
	zyskkoncowy = wyniki[liczbaprzedmiotow][pojemnoscplecaka];
	while (indeks>0){ //zaczynamy od ostatniego indeksu i idziemy w gore
		if (przedmioty[indeks][tempwaga] == 1){ // == 1 => w plecaku
			cout << waga[indeks - 1] << "\t" << wartosc[indeks - 1] << "\t\t" << wspprzedmiotu[indeks - 1] << endl;
			wagak += waga[indeks - 1];
			tempwaga -= waga[indeks - 1]; //'waga przedmiotu' kolumn w lewo
			indeks--; //jeden wiersz w gore
		}
		else{ //przedmiot poza plecakiem
			indeks--; //jeden wiersz w gore
		}
	}
	cout << "\nMaksymalny zysk: " << zyskkoncowy << ", waga plecaka: " << wagak << endl;
}
//---------------------------------------------------------------------------------------

//FPTAS
void FPTAS(int ileprzedmiotow, int rozmiar)
{
	int i, j;

	//szukamy w tablicy o indeksach: [przedmioty][wartosci] minimalnej wagi potrzebnej do uzyskania [wartosci] majac do dyspozycji [przedmioty] przedmiotow. 
	for (i = 1; i <= ileprzedmiotow; i++){ //i - indeksy przedmiotow
		for (j = 1; j <= temp * liczbaprzedmiotow; j++){ //j - wagi przedmiotow
			if ((j - wartosc2[i - 1]) < 0 || wynikiFPTAS[i - 1][j - wartosc2[i - 1]] == MAXINT) //indeks wartosc - wartosc przedmiotu > 0 lub wynik z wiersza wyzej i kolumny o wartosc w lewo == nieskonczonosc
			{
				wynikiFPTAS[i][j] = wynikiFPTAS[i - 1][j]; //wynik jest przepisywany z wiersza powyzej
			}
			else
			{
				wynikiFPTAS[i][j] = min(wynikiFPTAS[i - 1][j], wynikiFPTAS[i - 1][j - wartosc2[i - 1]] + waga[i - 1]); //minimum z poprzedniego wiersza(PW) lub (PW i kolumny o wartosc w lewo) + waga 
			}
			//Do testow
			/*if (wynikiFPTAS[liczbaprzedmiotow][j] <= pojemnoscplecaka)
			{
				zyskkoncowy = j;
			}*/
		}
	}
	//cout << zyskkoncowy << "\t"; //do testow
}


//przygotowanie wartosci K, obliczonej na podstawie najwiekszej wagi, ilosci przedmiotow i epsilon
void maxWaga()
{

	for (int i = 0; i < liczbaprzedmiotow; i++) //wyszukiwanie max. wartosci z zestawu
	{
		if (wartosc[i] > temp) temp = wartosc[i];
	}

	if (temp <= 0) //wartosc ujemna/==0 
	{
		cout << "Brak przedmiotow" << endl;
		return;
	}
	else K = (float)((epsilon * (float)temp) / (float)liczbaprzedmiotow);

	if (K < 1) K = 1; //w razie gdyby K wyszlo < 1, zeby tablica sie nie powieszkyla bezsensownie

	for (int i = 0; i < liczbaprzedmiotow; i++) //obliczanie nowych, przeskalowanych wartosci i wspolczynnikow
	{
		wartosc2[i] = floor(((float)wartosc[i] / K)); //floor na wszelki wypadek, konwersja float -> int powinna zrobic floor z automatu
		wspprzedmiotu[i] = (float)wartosc2[i] / (float)waga[i];
	}

	temp = (int)(temp / K); //skalujemy tez max. wartosc

	//rezerwowanie miejsca na tablice wynikow
	wynikiFPTAS = new  int*[liczbaprzedmiotow + 1];
	for (int i = 0; i <= liczbaprzedmiotow; i++)
	{
		wynikiFPTAS[i] = new int[(temp*liczbaprzedmiotow) + 1];
	}

	for (int i = 0; i <= liczbaprzedmiotow; i++) //zerowanie pierwszej kolumny
	{
		wynikiFPTAS[i][0] = 0;
	}

	for (int i = 1; i <= (temp*liczbaprzedmiotow); i++) //=="infinity"(MAXINT, 2^31) dla 1 wiersza
	{
		wynikiFPTAS[0][i] = MAXINT;
	}

}

void wyswietlFPTAS()
{
	cout << "Zawartosc plecaka: " << endl << endl;
	cout << "Waga\t Wartosc(stara wartosc)\t Wspolczynnik\t" << endl;
	int sumWaga = 0;
	int sumWartosc = 0;
	int tempplecak = pojemnoscplecaka; //pozostale miejsce w plecaku

	for (int i = liczbaprzedmiotow * temp; i >= 0; i--) //zaczynamy od ostatniej kolumny
	{
		for (int j = 0; j <= liczbaprzedmiotow; j++) //i od pierwszego wiersza
		{
			if (wynikiFPTAS[j][i] <= tempplecak && wynikiFPTAS[j][i] != 0) //szukamy komorki != 0 i mieszczacej sie w plecaku
			{
				if ((i - wartosc2[j - 1]) >= 0) //sprawdzamy czy wartosc przedmiotu nie jest wieksza od indeksu kolumny (ktora oznacza wartosci)
				{
					tempplecak -= waga[j - 1]; //zabieramy miejsce z plecaka (pomniejszamy o wage)
/*FORMATOWANIE*/		if (wartosc2[j - 1] > 99 && wartosc[j - 1] > 99) cout << waga[j - 1] << "\t" << wartosc2[j - 1] << "(" << wartosc[j - 1] << ")\t\t" << wspprzedmiotu[j - 1] << endl; //dane przedmiotu z nowa wartoscia, w nawiasie stara
/*WYSWIETLANIA*/		else cout << waga[j - 1] << "\t" << wartosc2[j - 1] << "(" << wartosc[j - 1] << ")  \t\t" << wspprzedmiotu[j - 1] << endl;
					sumWaga += waga[j - 1];
					sumWartosc += wartosc2[j - 1];

					i -= wartosc2[j - 1]; //przesuwamy sie o wartosc przedmiotu w lewo
					j = 0; //wracamy do przegladania od 1 wiersza
				}
				else //nie miesci sie - koniec
				{
					break;
				}
			}
		}
	}

	cout << "\nMaksymalny zysk: " << sumWartosc << ", waga plecaka: " << sumWaga << ", epsilon:" << epsilon << ", K: " << K << endl;
}
//---------------------------------------------------------------------------------------

//PRZEGLAD ZUPELNY
//binarnie, w 1. petli k==1 przedmiot - w plecaku, w 2. petli k = indeks przedmiotu
//indeksy przedmiotow odpowiadaja indeksom wektora o wartosciach 0,1 np: [i=9, 8, 7, ... , 1, 0]
//sprawdzane sa kolejne wektory od [0, 0, ... , 0, 1] az do [1, 1, ... , 1, 1]
//maksymalne wartosci sa przechowywane w temp(Weight/Value)
//po zakonczeniu pracy w glownej petli ostateczny wynik (wektor) jest przypisywany do tablicy plecak[] (wykorzystywanej przy wyswietlaniu)
void przegladzupelny()
{
	int *A = (int *)calloc(liczbaprzedmiotow, sizeof(int));
	int i;

	for (i = 0;; i++) {
		int j = liczbaprzedmiotow;
		int tempWeight = 0;
		int tempValue = 0;
		int k;

		k = 1;
		for (j = 0; j < liczbaprzedmiotow; j++) {
			A[j] += k;

			k = A[j] / 2;
			A[j] = A[j] % 2;
		}
		if (k) break;

		for (k = 0; k < liczbaprzedmiotow; k++) {
			if (A[k] == 1) {
				tempWeight = tempWeight + waga[k];
				tempValue = tempValue + wartosc[k];
			}
		}

		if (tempValue > zyskkoncowy && tempWeight <= pojemnoscplecaka) {
			zyskkoncowy = tempValue;
			for (int i = 0; i < liczbaprzedmiotow; i++) plecak[i] = A[i];
		}
	}

	free(A); //zwolnienie pamieci

	for (int i = 0; i < liczbaprzedmiotow; i++)
		wspprzedmiotu[i] = (float)wartosc[i] / (float)waga[i]; //wspolczynniki wykorzystywane przy wyswietlaniu
}
//---------------------------------------------------------------

//GENERACJA LOSOWA PROBLEMU PLECAKOWEGO O ZADANYCH WARTOSCIACH
void random(/*int a, int b*/)
{
	//W razie gdyby wykonywano wiele algorytmow
	liczbaprzedmiotow = pojemnoscplecaka = wagakoncowa = i = j = 0;
	zyskkoncowy = -1;
	for (int i = 0; i < max; i++)
	{
		waga[i] = wartosc[i] = plecak[i] = tmp[i] = wspprzedmiotu[i] = 0;
	}
	//---------------------------------------------------------------------
	srand(time(NULL));


	cout << "Podaj ilosc przedmiotow: ";
	cin >> liczbaprzedmiotow;
	cout << endl;

	cout << "Podaj wage plecaka: ";
	cin >> pojemnoscplecaka;
	cout << endl;

	//liczbaprzedmiotow = a;
	//pojemnoscplecaka = a*3;

	for (int i = 0; i < liczbaprzedmiotow; i++)
	{
		wartosc[i] = (rand() % (int)(0.75*100)) + (100 - (0.75*100));
		//wartosc[i] = (rand() % (int)(0.75*b)) + (b-(0.75*b));
		do {
			waga[i] = (rand() % 10) + 1;
		} while (waga[i] > wartosc[i]);
	}
}
//---------------------------------------------------------

//MENU PROGRAMU
int main()
{
	string nazwa;
	int wybor;
	do
	{
		cout << "1 - Wczytaj z pliku \n2 - Wyswietl strukture\n3 - Branch&Bound\n4 - Dynamiczne\n5 - Przeglad zupelny\n6 - FPTAS\n7 - Generacja losowa\n8 - Koniec" << endl;
		cin >> wybor;
		system("cls");
		switch (wybor)
		{
		case 1: {
			string nazwa;
			cout << "Podaj nazwe pliku: ";
			cin >> nazwa;
			zpliku(nazwa);
			break;
		}
		case 2: struktura(); break;
		case 3: {
			wyczysc();
			QueryPerformanceFrequency(&freq); //pobieranie czêstotliwoœci pracy procesora
			performanceCountStart = startTimer(); //czas start
			sortuj();
			doplecaka(0, 0.0, 0.0);
			performanceCountEnd = endTimer(); //czas stop
			double tm = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
			cout << "\nZrobione! Czas wykonania: " << ((tm / freq.QuadPart * 1000) * 1000) << " mikrosekund" << endl;
			wyswietl();
			_getch();
			system("cls");
			break;
		}
		case 4: {
			wyczysc();
			QueryPerformanceFrequency(&freq); //pobieranie czêstotliwoœci pracy procesora
			performanceCountStart = startTimer(); //czas start
			dynamiczne(liczbaprzedmiotow, pojemnoscplecaka);
			performanceCountEnd = endTimer(); //czas stop
			double tm = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
			cout << "\nZrobione! Czas wykonania: " << ((tm / freq.QuadPart * 1000) * 1000) << " mikrosekund" << endl;
			wyswietldynamiczne();
			_getch();
			system("cls");
			break; }
		case 5: {
			wyczysc();
			QueryPerformanceFrequency(&freq); //pobieranie czêstotliwoœci pracy procesora
			performanceCountStart = startTimer(); //czas start
			przegladzupelny();
			performanceCountEnd = endTimer(); //czas stop
			double tm = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
			cout << "\nZrobione! Czas wykonania: " << ((tm / freq.QuadPart * 1000) * 1000) << " mikrosekund" << endl;
			wyswietl();
			_getch();
			system("cls");
			break; }
		case 6: {
			wyczysc();
			cout << "Podaj wartosc epsilon: ";
			cin >> epsilon;
			maxWaga();
			QueryPerformanceFrequency(&freq); //pobieranie czêstotliwoœci pracy procesora
			performanceCountStart = startTimer(); //czas start
			FPTAS(liczbaprzedmiotow, pojemnoscplecaka);
			performanceCountEnd = endTimer(); //czas stop
			double tm = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
			cout << "\nZrobione! Czas wykonania: " << ((tm / freq.QuadPart * 1000) * 1000) << " mikrosekund" << endl;
			wyswietlFPTAS();
			_getch();
			system("cls");
			break;
		}
		case 7: {
			random(/*6, 10*/);
			_getch();
			system("cls");
			break; }
		case 8: break;

		//opcje do testowania FPTAS
		/*
		case 9: {
			cout << "=SUMA(C3:C102)/100\t=SUMA(D3:D102)/100" << endl; //1 wiersz, do Excela (liczenie sredniej)
			for (int i = 0; i < 15; i++) //100 iteracji do porownania
			{
				wyczysc();
				epsilon = 0.1;
				random(50, 500);
				maxWaga();
				QueryPerformanceFrequency(&freq); //pobieranie czêstotliwoœci pracy procesora
				performanceCountStart = startTimer(); //czas start
				FPTAS(liczbaprzedmiotow, pojemnoscplecaka);
				performanceCountEnd = endTimer(); //czas stop
				double tm = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
				cout << ((tm / freq.QuadPart * 1000) * 1000) << "\t";

				performanceCountStart = startTimer(); //czas start
				dynamiczne(liczbaprzedmiotow, pojemnoscplecaka);
				performanceCountEnd = endTimer(); //czas stop
				double tm2 = (performanceCountEnd.QuadPart - performanceCountStart.QuadPart); //obliczenie wyniku
				cout << ((tm2 / freq.QuadPart * 1000) * 1000) << endl;
			}
			_getch();
			system("cls");
			break;
		}
		case 0: { //przed uruchomieniem - sprawdz czy algorytmy wyswietlaja
			int a = 5, b = 37;
			for (int i = 0; i < 15; i++) //100 iteracji do porownania
			{
				wyczysc();
				epsilon = 0.20;
				random(a, b);
				maxWaga();
				FPTAS(liczbaprzedmiotow, pojemnoscplecaka);

				dynamiczne(liczbaprzedmiotow, pojemnoscplecaka);
				a += 5; b += 43;
			}
			_getch();
			system("cls");
			break;
		}*/
		default: cout << "Podano zly znak" << endl; break;
		}
	} while (wybor != 8);
}
