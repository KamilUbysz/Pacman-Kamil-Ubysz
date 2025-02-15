#include <SFML/Graphics.hpp> 
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace sf;

const float TILE_SIZE = 32.0f;
const int WIDTH = 20;
const int HEIGHT = 15;
const float PLAYER_SPEED = 100.0f;
const int CANDY_POINTS = 100;
const int COIN_POINTS = 1000;
int MAX_OBSTACLES;
float OBSTACLE_SPAWN_INTERVAL;
float OBSTACLE_SPEED;

class Punkt
{
public:
    int x, y;
    Punkt(int x = 0, int y = 0) : x(x), y(y) {}
};

class Obstacle 
{
public:
    Punkt pozycja;
    Punkt kierunek;
    Vector2f aktualnaPozycja;
    Vector2f cel;
    bool zjadliwy;

    Obstacle(int x, int y) :
        pozycja(x, y),
        aktualnaPozycja(x* TILE_SIZE, y* TILE_SIZE),
        cel(aktualnaPozycja),
        zjadliwy(false)
    {
        ustawKierunek();
    }

    void ustawKierunek() 
    {
        kierunek = Punkt((rand() % 2) * 2 - 1, 0); 
        if (rand() % 2) kierunek = Punkt(0, (rand() % 2) * 2 - 1); 
    }

    void ustawCel(int nowyX, int nowyY) 
    {
        pozycja.x = nowyX;
        pozycja.y = nowyY;
        cel = Vector2f(pozycja.x * TILE_SIZE, pozycja.y * TILE_SIZE);
    }

    void aktualizuj(vector<vector<int>>& plansza, float deltaTime) 
    {
        Vector2f ruch = cel - aktualnaPozycja;
        float odleglosc = sqrt(ruch.x * ruch.x + ruch.y * ruch.y);
        if (odleglosc > 1.0f)
        {
            ruch /= odleglosc;
            aktualnaPozycja += ruch * OBSTACLE_SPEED * deltaTime;
        }
        else
        {
            aktualnaPozycja = cel;
            int nowyX = pozycja.x + kierunek.x;
            int nowyY = pozycja.y + kierunek.y;

            if (nowyX < 0 || nowyX >= WIDTH || nowyY < 0 || nowyY >= HEIGHT || plansza[nowyY][nowyX] == 1)
            {
                ustawKierunek();
            }
            else {
                ustawCel(nowyX, nowyY);
            }
        }
        if (zegarBonusu.getElapsedTime().asSeconds() > 10)
        {
            OBSTACLE_SPEED *= 2.0;
        }

    }
};

class Gracz
{
public:
    Punkt pozycja;
    Vector2f aktualnaPozycja;
    Vector2f cel;
    float katRotacji = 0.0f; 
    bool wRuchu;
    bool zjadaPrzeszkody;

    Gracz(int x, int y) : pozycja(x, y), aktualnaPozycja(x* TILE_SIZE, y* TILE_SIZE), cel(aktualnaPozycja), wRuchu(false), zjadaPrzeszkody(false) {}

    void ustawCel(int dx, int dy, vector<vector<int>>& plansza) 
    {
        int nowyX = pozycja.x + dx;
        int nowyY = pozycja.y + dy;

        if (!wRuchu && nowyX >= 0 && nowyX < WIDTH && nowyY >= 0 && nowyY < HEIGHT && plansza[nowyY][nowyX] != 1) 
        {
            pozycja.x = nowyX;
            pozycja.y = nowyY;
            cel = Vector2f(pozycja.x * TILE_SIZE, pozycja.y * TILE_SIZE);
            wRuchu = true;

            
            if (dx == 1 && dy == 0) katRotacji = 0.0f;       
            else if (dx == -1 && dy == 0) katRotacji = 180.0f; 
            else if (dx == 0 && dy == -1) katRotacji = 270.0f; 
            else if (dx == 0 && dy == 1) katRotacji = 90.0f; 
        }
    }


    void aktualizuj(float deltaTime)
    {
        if (wRuchu)
        {
            Vector2f kierunek = cel - aktualnaPozycja;
            float odleglosc = sqrt(kierunek.x * kierunek.x + kierunek.y * kierunek.y);
            if (odleglosc > 1.0f) 
            {
                kierunek /= odleglosc;
                aktualnaPozycja += kierunek * PLAYER_SPEED * deltaTime;
            }
            else 
            {
                aktualnaPozycja = cel;
                wRuchu = false;
            }
        }
    }
};

class Candy
{
public:
    Punkt pozycja;
    bool aktywny;

    Candy() : pozycja(-1, -1), aktywny(false) {}

    void generuj(vector<vector<int>>& plansza)
    {
        do
        {
            pozycja.x = rand() % WIDTH;
            pozycja.y = rand() % HEIGHT;
        }
        while (plansza[pozycja.y][pozycja.x] != 0);
        aktywny = true;
    }

    void zbierz()
    {
        aktywny = false;
        pozycja = Punkt(-1, -1);
    }
};

class Coin
{
public:
    Punkt pozycja;
    bool aktywny;

    Coin() : pozycja(-1, -1), aktywny(false) {}

    void generuj(vector<vector<int>>& plansza)
    {
        do 
        {
            pozycja.x = rand() % WIDTH;
            pozycja.y = rand() % HEIGHT;
        } while (plansza[pozycja.y][pozycja.x] != 0);
        aktywny = true;
    }

    void zbierz()
    {
        aktywny = false;
        pozycja = Punkt(-1, -1);
    }
};

class Bonus
{
public:
    Punkt pozycja;
    bool aktywny;
    int typ;
    Clock zegarBonusu;

    Bonus() : pozycja(-1, -1), aktywny(false), typ(0) {}

    void generuj(vector<vector<int>>& plansza) 
    {
        do
        {
            pozycja.x = rand() % WIDTH;
            pozycja.y = rand() % HEIGHT;
        } 
        while (plansza[pozycja.y][pozycja.x] != 0);

        aktywny = true;
        typ = rand() % 3; 
        zegarBonusu.restart();
    }

    void zbierz()
    {
        aktywny = false;
        pozycja = Punkt(-1, -1);
    }
   
};


class Gra
{
private:
    RenderWindow okno;
    vector<vector<int>> plansza;
    Gracz gracz;
    Texture teksturaSciany, teksturaGracza, teksturaCukierka, teksturaPrzeszkody, teksturaMonety, teksturaBonusZycie, teksturaBonusZwolnienie, teksturaBonusCzas;
    Sprite sciana, postac, cukierekSprite, przeszkodaSprite, monetaSprite, bonusSprite;
    vector<Obstacle> przeszkody;
    vector<Candy> cukierki;
    Coin moneta;
    int punkty = 0;
    bool poziomSkonczony = false;
    string komunikat;
    Clock zegarPrzeszkod, zegarCzasu;
    int liczbaPrzeszkod = 0;
    int zycia = 3; 
    const int maksymalnyCzas = 110; 
    Font font;
    bool graZapauzowana = false;
    bool wMenu = true; 
    int poziomTrudnosci = 1; 
    bool monetaZebrana = false; 
    bool graPrzegrana = false;
    string playerName; 
    bool wpisywanieImienia = true; 
    bool graSkonczona = false;
    bool kontynuujGre = false; 
    bool wyborWToku = false;   
    int menuWybor = 0;         
    float czasPauzy = 0.0f; 
    bool potwierdzWyjscie = false; 
    int potwierdzenieWybor;
    Bonus bonus;
    Text komunikatBonus;
    Clock zegarKomunikatu;
    bool pokazKomunikatBonus = false;



    void utworzPlansze()
    {
        plansza = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
            {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
        };
    }


    void ladujTekstury()
    {
        if (!teksturaSciany.loadFromFile("wall.png") ||
            !teksturaGracza.loadFromFile("player.png") ||
            !teksturaCukierka.loadFromFile("candy.png") ||
            !teksturaPrzeszkody.loadFromFile("obstacle.png") ||
            !teksturaMonety.loadFromFile("coin.png") ||
            !teksturaBonusZycie.loadFromFile("bonus_zycie.png") ||
            !teksturaBonusZwolnienie.loadFromFile("bonus_zwolnienie.png") ||
            !teksturaBonusCzas.loadFromFile("bonus_czas.png") ||
            !font.loadFromFile("ChamsBold.ttf")) 
        {
            cerr << "Blad ladowania zasobow!" << endl;
            exit(EXIT_FAILURE);
        }
        sciana.setTexture(teksturaSciany);
        postac.setTexture(teksturaGracza);
        cukierekSprite.setTexture(teksturaCukierka);
        przeszkodaSprite.setTexture(teksturaPrzeszkody);
        monetaSprite.setTexture(teksturaMonety);
        postac.setOrigin(teksturaGracza.getSize().x / 2.0f, teksturaGracza.getSize().y / 2.0f);


    }

    void spawnujPrzeszkody()
    {
        if (liczbaPrzeszkod < MAX_OBSTACLES && zegarPrzeszkod.getElapsedTime().asSeconds() >= OBSTACLE_SPAWN_INTERVAL)
        {
            int x, y;
            bool poprawneMiejsce = false;

           
            while (!poprawneMiejsce)
            {
                x = rand() % WIDTH; 
                y = rand() % HEIGHT; 

                int dx = abs(x - gracz.pozycja.x);
                int dy = abs(y - gracz.pozycja.y);


                if (plansza[y][x] == 0 && dx >= 5 && dy >= 5)
                {
                    poprawneMiejsce = true;
                }
            }

            
            przeszkody.emplace_back(x, y);
            liczbaPrzeszkod++;
            zegarPrzeszkod.restart();

            
            if (liczbaPrzeszkod == 5)
            {
                for (auto& cukierek : cukierki)
                {
                    if (!cukierek.aktywny)
                    {
                        cukierek.generuj(plansza);
                        cukierek.aktywny = true;
                    }
                }
            }
        }
    }




    void sprawdzKolizje() 
    {
       
        bool wszystkieCukierkiZebrane = true;

        for (auto& cukierek : cukierki)
        {
            if (cukierek.aktywny)
            {
                wszystkieCukierkiZebrane = false;
                break;
            }
        }

       
        if (wszystkieCukierkiZebrane && liczbaPrzeszkod >= MAX_OBSTACLES && !moneta.aktywny && !monetaZebrana)
        {
            moneta.generuj(plansza);
            moneta.aktywny = true;
        }

        if (punkty % 50 == 0 && punkty > 0 && !bonus.aktywny)
        {
            bonus.generuj(plansza);
            bonus.aktywny = true;
            cout << "Bonus aktywny " << endl;
        }

        
        for (auto& cukierek : cukierki) 
        {
            if (cukierek.aktywny && gracz.pozycja.x == cukierek.pozycja.x && gracz.pozycja.y == cukierek.pozycja.y)
            {
                cukierek.zbierz();
                punkty += 10;
            }
        }

        if (moneta.aktywny && gracz.pozycja.x == moneta.pozycja.x && gracz.pozycja.y == moneta.pozycja.y)
        {
            moneta.zbierz();
            punkty += 50;
            gracz.zjadaPrzeszkody = true;
            monetaZebrana = true;
        }


        for (size_t i = 0; i < przeszkody.size(); ++i)
        {
            if (przeszkody[i].pozycja.x == gracz.pozycja.x && przeszkody[i].pozycja.y == gracz.pozycja.y)
            {
                if (gracz.zjadaPrzeszkody)
                {
                    przeszkody.erase(przeszkody.begin() + i);
                    punkty += 20;
                    --i;
                }
                else
                {
                    przeszkody.erase(przeszkody.begin() + i);
                    --i;
                    if (punkty >= 30)
                    {
                        punkty -= 30;
                    }
                    else
                    {
                        punkty = 0;
                    }
                    zycia--;

                    return;
                }
            }
        }
       

        if (bonus.aktywny && gracz.pozycja.x == bonus.pozycja.x && gracz.pozycja.y == bonus.pozycja.y)
        {
            bonus.zbierz();

            switch (bonus.typ)
            {
            case 0: 
                zycia++;
                komunikatBonus.setString("Bonus: Dodatkowe zycie!");
                break;
            case 1: 
                OBSTACLE_SPEED *= 0.5;
                komunikatBonus.setString("Bonus: Zwolnienie przeszkod!");
                zegarBonusu.restart();
                break;
            case 2: 
                czasPauzy -= 15;
                komunikatBonus.setString("Bonus: +15 sekund!");
                break;
            }
            punkty += 10;
            pokazKomunikatBonus = true;
            zegarKomunikatu.restart();
        }
    }


    void rysujUI()
    {
        int czasGry = czasPauzy + (graZapauzowana ? 0 : zegarCzasu.getElapsedTime().asSeconds());
        if (graPrzegrana || poziomSkonczony) return;
        Text text;
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color::White);



        stringstream ss;
        ss << "Punkty: " << punkty << "\n";
        ss << "Zycia: " << zycia << "\n";
        ss << "Czas: " << max(0, maksymalnyCzas - czasGry) << " s\n";
        ss << "Poziom trudnosci: " << poziomTrudnosci << "\n";
        ss << "Pomoc : F1\n";


        text.setString(ss.str());
        text.setPosition(WIDTH * TILE_SIZE + 10, 10); 
        okno.draw(text);
    }


    void rysuj()
    {
        okno.clear();
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                if (plansza[y][x] == 1)
                {
                    sciana.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    okno.draw(sciana);
                }
            }
        }

        for (auto& cukierek : cukierki)
        {
            if (cukierek.aktywny)
            {
                cukierekSprite.setPosition(cukierek.pozycja.x * TILE_SIZE, cukierek.pozycja.y * TILE_SIZE);
                okno.draw(cukierekSprite);
            }
        }

        if (moneta.aktywny)
        {
            monetaSprite.setPosition(moneta.pozycja.x * TILE_SIZE, moneta.pozycja.y * TILE_SIZE);
            okno.draw(monetaSprite);
        }

        for (auto& przeszkoda : przeszkody)
        {
            przeszkodaSprite.setPosition(przeszkoda.aktualnaPozycja);
            okno.draw(przeszkodaSprite);
        }

        postac.setPosition(gracz.aktualnaPozycja.x + TILE_SIZE / 2.0f, gracz.aktualnaPozycja.y + TILE_SIZE / 2.0f);
        postac.setRotation(gracz.katRotacji);
        okno.draw(postac);



        rysujUI();
        if (zycia <= 0 || zegarCzasu.getElapsedTime().asSeconds() >= maksymalnyCzas) {
            graPrzegrana = true;
        }

        if (przeszkody.empty() && monetaZebrana)
        {
            poziomSkonczony = true;
        }

        if (poziomSkonczony)
        {
            okno.clear();
            Text tekst;
            tekst.setFont(font);
            tekst.setCharacterSize(24);
            tekst.setFillColor(Color::White);

            if (poziomTrudnosci < 3) {
                tekst.setString("Poziom ukonczony! \nNacisnij Enter, aby przejsc do wyzszego poziomu.");
            }
            else {
                tekst.setString("Gratulacje! Ukonczyles gre.\n Tw�j wynik: " + to_string(punkty) + " punktow.\n Nacisnij Enter, aby zagrac od nowa.");
            }

            tekst.setPosition(WIDTH * TILE_SIZE / 4, HEIGHT * TILE_SIZE / 2);
            okno.draw(tekst);
            okno.display();

            return;
        }
        if (graPrzegrana)
        {
            okno.clear();
            Text tekst;
            tekst.setFont(font);
            tekst.setCharacterSize(24);
            tekst.setFillColor(Color::White);
            if (zycia <= 0) {
                tekst.setString("Przegrales " + playerName + "! Skonczyly ci sie zycia.\n Tw�j wynik: " + to_string(punkty) + " punktow.\n Nacisnij escape, aby wrocic do menu");


            }
            if (zegarCzasu.getElapsedTime().asSeconds() >= maksymalnyCzas) {
                tekst.setString("Przegrales " + playerName + "! Skonczyl sie czas.\n Tw�j wynik: " + to_string(punkty) + " punktow.\n Nacisnij escape, aby wrocic do menu");

            }
            tekst.setPosition(WIDTH * TILE_SIZE / 4, HEIGHT * TILE_SIZE / 2);
            okno.draw(tekst);
        }
        if (bonus.aktywny)
        {
            switch (bonus.typ)
            {
            case 0: bonusSprite.setTexture(teksturaBonusZycie); break;
            case 1: bonusSprite.setTexture(teksturaBonusZwolnienie); break;
            case 2: bonusSprite.setTexture(teksturaBonusCzas); break;
            }
            bonusSprite.setPosition(bonus.pozycja.x * TILE_SIZE, bonus.pozycja.y * TILE_SIZE);
            okno.draw(bonusSprite);
        }


        
        if (pokazKomunikatBonus && zegarKomunikatu.getElapsedTime().asSeconds() < 1)
        {
            okno.draw(komunikatBonus);
        }
        else
        {
            pokazKomunikatBonus = false;
        }

        okno.display();
    }

    void rysujPauze() 
    {
        okno.clear();

        Text text;
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(Color::White);
        text.setString("Pauza - nacisnij spacje, aby kontynuowac");
        text.setPosition(WIDTH * TILE_SIZE / 4, HEIGHT * TILE_SIZE / 2);

        okno.draw(text);
        okno.display();
    }

    void rysujMenu()
    {
        okno.clear();
        Text instrukcja;
        instrukcja.setFont(font);
        instrukcja.setCharacterSize(24);
        instrukcja.setFillColor(Color::White);
        
        if (potwierdzWyjscie)
        {
            string opcjePotwierdzenia;
            opcjePotwierdzenia += (potwierdzenieWybor == 0 ? "-> Tak, wyjdz z gry\n" : "   Tak, wyjdz z gry\n");
            opcjePotwierdzenia += (potwierdzenieWybor == 1 ? "-> Nie, wroc do menu\n" : "   Nie, wroc do menu\n");


            instrukcja.setString("Czy na pewno chcesz wyjsc?\nWszystkie postepy zostana utracone!\n\n" + opcjePotwierdzenia);
        }

        else if (wpisywanieImienia)
        {
            instrukcja.setString("Podaj swoje imie: " + playerName + "_\n");
        }
        else 
        {
            string opcjeMenu;

            if (kontynuujGre)
            {
                opcjeMenu += (menuWybor == 0 ? "-> Kontynuuj\n" : "   Kontynuuj\n");
                opcjeMenu += (menuWybor == 1 ? "-> Nowa gra\n" : "   Nowa gra\n");
                opcjeMenu += (menuWybor == 2 ? "-> Cofnij do menu startowego\n" : "   Cofnij do menu startowego\n");
            }
            else
            {
                opcjeMenu = "Nacisnij Enter, aby rozpoczac nowa gre.\n";
            }

            instrukcja.setString("Witaj, " + playerName + "!\n" + opcjeMenu +
                "Aby zobaczyc ranking najlepszych graczy wcisnij F2.");
        }

        instrukcja.setPosition(WIDTH * TILE_SIZE / 4, HEIGHT * TILE_SIZE / 3);
        okno.draw(instrukcja);
        okno.display();
    }



    void ustawParametryTrudnosci()
    {
        switch (poziomTrudnosci)
        {
        case 1: 
            MAX_OBSTACLES = 10;
            OBSTACLE_SPEED = 32.0f;
            OBSTACLE_SPAWN_INTERVAL = 5.0f;
            break;
        case 2:
            MAX_OBSTACLES = 15;
            OBSTACLE_SPEED = 50.0f;
            OBSTACLE_SPAWN_INTERVAL = 3.0f;
            break;
        case 3: 
            MAX_OBSTACLES = 20;
            OBSTACLE_SPEED = 70.0f;
            OBSTACLE_SPAWN_INTERVAL = 2.0f;
            break;
        }
    }

    void resetujGre()
    {

        
            utworzPlansze();
            gracz = Gracz(1, 1);
            przeszkody.clear();
            liczbaPrzeszkod = 0;
            for (auto& cukierek : cukierki) 
            {
                cukierek.aktywny = false;
                cukierek.pozycja = Punkt(-1, -1);
            }
            moneta.aktywny = false;
            moneta.pozycja = Punkt(-1, -1);
            zycia = 3;
            gracz.zjadaPrzeszkody = false;
            zegarCzasu.restart();
            graPrzegrana = false;
            graSkonczona = false;
            poziomSkonczony = false;
            monetaZebrana = false;
            graZapauzowana = false;


        ustawParametryTrudnosci();


    }


    vector<pair<string, int>> wczytajRanking() {
        ifstream plik("ranking.txt");
        vector<pair<string, int>> ranking;
        string imie;
        int wynik;

        if (plik.is_open()) {
            while (plik >> imie >> wynik)
            {
                ranking.push_back({ imie, wynik });
            }
            plik.close();
        }

        
        sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b)
            {return a.second > b.second;});

        return ranking;
    }

    void zapiszWynik() {
        vector<pair<string, int>> ranking = wczytajRanking(); 
        bool znaleziono = false;

        
        for (auto& rekord : ranking)
        {
            if (rekord.first == playerName)
            {
                if (punkty > rekord.second)
                {
                    rekord.second = punkty; 
                }
                znaleziono = true;
                break;
            }
        }

        
        if (!znaleziono) 
        {
            ranking.push_back({ playerName, punkty });
        }

        
        sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b) 
            {
            return a.second > b.second;
            });

        
        ofstream plik("ranking.txt");
        if (plik.is_open()) 
        {
            for (const auto& rekord : ranking) 
            {
                plik << rekord.first << " " << rekord.second << endl;
            }
            plik.close();
        }
    }


    void wyswietlRankingOkno() {
        RenderWindow rankingOkno(VideoMode(400, 500), "Ranking najlepszych graczy");

        vector<pair<string, int>> ranking = wczytajRanking();

        Font rankingFont;
        if (!rankingFont.loadFromFile("ChamsBold.ttf"))
        {
            cerr << "Blad ladowania czcionki!" << endl;
            return;
        }

        while (rankingOkno.isOpen()) {
            Event event;
            while (rankingOkno.pollEvent(event))
            {
                if (event.type == Event::Closed) 
                {
                    rankingOkno.close();
                }
            }

            rankingOkno.clear(Color::Black);

            Text naglowek("Ranking najlepszych graczy", rankingFont, 24);
            naglowek.setFillColor(Color::White);
            naglowek.setPosition(20, 20);
            rankingOkno.draw(naglowek);

            for (size_t i = 0; i < min(ranking.size(), size_t(10)); i++)
            {
                Text tekst;
                tekst.setFont(rankingFont);
                tekst.setCharacterSize(20);
                tekst.setFillColor(Color::White);
                tekst.setString(to_string(i + 1) + ". " + ranking[i].first + " - " + to_string(ranking[i].second) + " pkt");
                tekst.setPosition(30, 60 + i * 30);
                rankingOkno.draw(tekst);
            }

            rankingOkno.display();
        }
    }
    void wyswietlPomocOkno()
    {
        RenderWindow pomocOkno(VideoMode(500, 400), "Pomoc - Zasady gry i sterowanie");

        Font pomocFont;
        if (!pomocFont.loadFromFile("ChamsBold.ttf"))
        {
            cerr << "Blad ladowania czcionki!" << endl;
            return;
        }
       
        while (pomocOkno.isOpen())
        {
            Event event;
            while (pomocOkno.pollEvent(event))
            {
                if (event.type == Event::Closed) 
{
                    pomocOkno.close();
                }
            }

            pomocOkno.clear(Color::Black);

            Text naglowek("Witaj w grze PACMAN", pomocFont, 22);
            naglowek.setFillColor(Color::White);
            naglowek.setPosition(20, 20);
            pomocOkno.draw(naglowek);

            
              
               

            Text tekstPomocy;
            tekstPomocy.setFont(pomocFont);
            tekstPomocy.setCharacterSize(20);
            tekstPomocy.setFillColor(Color::White);
            tekstPomocy.setPosition(20, 60);
            tekstPomocy.setString("Zasady gry:\n"
                "  - Zbieraj cukierki, aby pojawila sie moneta.\n"
                "  - Zbierz monete, by moc niszczyc przeszkody.\n"
                "  - Unikaj przeszkod, tracisz za nie zycia.\n"
                "  - Masz 3 zycia i limit czasu na poziom.\n"
                "  - Po wygranej gra przechodzi na trudniejszy poziom.\n"

                "Sterowanie:\n"
                "  - W/A/S/D: ruch gracza\n"
                "  - Spacja: pauza\n"
                "  - Escape: powrot do menu\n"
                "  - F1: otworz pomoc\n"

                "Punktacja:\n"
                " - Zebranie cukierka: +10 pkt\n"
                " - Uderzenie w przeszkode: -30 pkt\n"
                " - Zebranie monety: +50 pkt\n"
                " - Zebranie przeszkody: +20 pkt");

            VertexArray przeszkoda(LineStrip, 6);

            przeszkoda[0].position = Vector2f(320, 250);
            przeszkoda[1].position = Vector2f(360, 240);
            przeszkoda[2].position = Vector2f(380, 260);
            przeszkoda[3].position = Vector2f(370, 290);
            przeszkoda[4].position = Vector2f(330, 280);
            przeszkoda[5].position = Vector2f(320, 250);

            przeszkoda[0].color = Color::Green;
            przeszkoda[1].color = Color::Green;
            przeszkoda[2].color = Color::Green;
            przeszkoda[3].color = Color::Green;
            przeszkoda[4].color = Color::Green;
            przeszkoda[5].color = Color::Green;

            pomocOkno.draw(przeszkoda);
            pomocOkno.draw(tekstPomocy);
            pomocOkno.display();
        }
    }



public:
    Gra() : okno(VideoMode(WIDTH* TILE_SIZE + 200, HEIGHT* TILE_SIZE), "Gra"), gracz(1, 1) 
    {
        srand(static_cast<unsigned>(time(0)));
        utworzPlansze();
        ladujTekstury();
        poziomTrudnosci = 1;
        zegarCzasu.restart();
        for (int i = 0; i < 7; ++i) 
        {
            Candy cukierek;
            cukierek.aktywny = false;
            cukierki.push_back(cukierek);
        }
        komunikatBonus.setFont(font);
        komunikatBonus.setCharacterSize(24);
        komunikatBonus.setFillColor(Color::Yellow);
        komunikatBonus.setPosition(WIDTH * TILE_SIZE / 4, HEIGHT * TILE_SIZE / 2);


    }

    void uruchom() 
    {
        Clock zegar;

        while (okno.isOpen()) 
        {
            Event event;
            while (okno.pollEvent(event))
            {
                if (event.type == Event::Closed)
                {
                    okno.close();
                }


                if (wMenu) 
                {
                    if (potwierdzWyjscie)
                    {
                        if (event.type == Event::KeyPressed)
                        {
                            if (event.key.code == Keyboard::Up)
                            {
                                potwierdzenieWybor = (potwierdzenieWybor > 0) ? potwierdzenieWybor - 1 : 1;
                            }
                            else if (event.key.code == Keyboard::Down)
                            {
                                potwierdzenieWybor = (potwierdzenieWybor < 1) ? potwierdzenieWybor + 1 : 0;
                            }
                            else if (event.key.code == Keyboard::Enter)
                            {
                                if (potwierdzenieWybor == 0)
                                {
                                    resetujGre();
                                    potwierdzWyjscie = false;
                                    wpisywanieImienia = true;
                                }
                                else if (potwierdzenieWybor == 1)
                                {
                                    potwierdzWyjscie = false;
                                    wMenu = true;
                                }
                            }
                        }
                    }

                    else if (wpisywanieImienia) 
                        if (event.type == Event::TextEntered)
                        {
                            {
                                if (event.text.unicode == '\b' && !playerName.empty())
                                {
                                    playerName.pop_back();
                                }

                                else if (event.text.unicode < 128 && playerName.size() < 10)
                                {
                                    playerName += static_cast<char>(event.text.unicode);
                                } 
                            }
                        }
                    if (event.type == Event::KeyPressed)
                    {
                        if (event.key.code == Keyboard::F2) 
                        {
                            wyswietlRankingOkno(); 
                        }
                        else if (wpisywanieImienia && event.key.code == Keyboard::Enter)
                        {
                            wpisywanieImienia = false; 
                        }
                        else if (!wpisywanieImienia && kontynuujGre)
                        {
                            if (event.key.code == Keyboard::Up)
                            {
                                menuWybor = (menuWybor > 0) ? menuWybor - 1 : 2;
                            }
                            else if (event.key.code == Keyboard::Down)
                            {
                                menuWybor = (menuWybor < 2) ? menuWybor + 1 : 0;
                            }
                            else if (event.key.code == Keyboard::Enter) 
                            {
                                if (menuWybor == 0) 
                                {
                                    wMenu = false; 
                                    graZapauzowana = false;
                                }
                                else if (menuWybor == 1) 
                                {
                                    resetujGre();
                                    punkty = 0;
                                    kontynuujGre = false;
                                    wMenu = false;
                                }
                                else if (menuWybor == 2) 
                                {
                                    potwierdzWyjscie = true;  
                                }
                            }
                        }
                        else if (!wpisywanieImienia && event.key.code == Keyboard::Enter)
                        {
                            resetujGre(); 
                            kontynuujGre = false;
                            wMenu = false;
                        }
                    }
                }
                else 
                {
                    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) 
                    {
                        graZapauzowana = !graZapauzowana; 

                        if (graZapauzowana)
                        {
                            czasPauzy += zegarCzasu.getElapsedTime().asSeconds();
                        }
                        else 
                        {
                            zegarCzasu.restart();
                        }
                    }
                    if (event.type == Event::KeyPressed && event.key.code == Keyboard::F1)
                    {
                        wyswietlPomocOkno();
                        graZapauzowana = true;
                    }
                    

                    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                    {
                        graZapauzowana = true;
                        kontynuujGre = true;
                        wMenu = true;
                        menuWybor = 0;
                    }
                    if (poziomSkonczony)
                    {
                        if (poziomTrudnosci < 3)
                        {
                            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter)
                            {
                                poziomTrudnosci++;
                                resetujGre();
                                ustawParametryTrudnosci();
                                rysuj();
                            }
                        }
                        else
                        {
                            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                            {
                                graSkonczona = true;
                                wMenu = true;
                            }
                        }

                    }
                    if (graPrzegrana)
                    {
                       
                        if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                        {
                            wMenu = true; 
                            graPrzegrana = false;
                        }
                    }

                }


            }
            if (graPrzegrana || graSkonczona)
            {
                zapiszWynik();
            }


            if (wMenu) 
            {
                rysujMenu();
                continue;
            }
            if (graZapauzowana)
            {
                rysujPauze();
                
            }




            if (!gracz.wRuchu)
            {
                if (Keyboard::isKeyPressed(Keyboard::W)) gracz.ustawCel(0, -1, plansza);
                if (Keyboard::isKeyPressed(Keyboard::S)) gracz.ustawCel(0, 1, plansza);
                if (Keyboard::isKeyPressed(Keyboard::A)) gracz.ustawCel(-1, 0, plansza);
                if (Keyboard::isKeyPressed(Keyboard::D)) gracz.ustawCel(1, 0, plansza);
            }

           
            float deltaTime = zegar.restart().asSeconds();
            if (!graZapauzowana)
            {
                gracz.aktualizuj(deltaTime);
                for (auto& przeszkoda : przeszkody)
                {
                    przeszkoda.aktualizuj(plansza, deltaTime);
                }

                spawnujPrzeszkody();
                sprawdzKolizje();
                rysuj();
                ustawParametryTrudnosci();
            }
        }


    }
};

int main() 
{
    Gra gra;
    gra.uruchom();
    return 0;
}